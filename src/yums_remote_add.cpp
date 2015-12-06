/*
 * Copyright (C) 2015 midnightBITS
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "argparser.hpp"
#include "filesystem.hpp"
#include "yums_db.hpp"
#include <http/uri.hpp>

#include <string>

namespace remote {
namespace add {

int call(args::parser& parser)
{
	bool verbose = false;
	std::string name;
	std::string href;
	parser.set<std::true_type>(verbose, "v").help("show more output").opt();
	parser.positional(name).meta("NAME").help("the nickname of the repo").req();
	parser.positional(href).meta("HREF").help("the address of the repo").req();
	parser.parse();

	if (name.empty())
		parser.error("argument NAME is required");

	if (href.empty())
		parser.error("argument HREF is required");

	yums_db db;
	if (!db.open_if_exists())
		parser.error("directory is not initialized", true);

	// href [/local/path -> file://localhost/local/path]
	// href [non-slash/resource -> http://non-slash/resource]
	auto repo = Uri { href };
	if (repo.relative()) {
		if (verbose)
			printf("-- turn href into absolute URL\n");
		fs::error_code ec;
		auto path = fs::path { href };
		if (fs::exists(path, ec) && !ec) {
			if (fs::is_directory(path, ec) && !ec) {
				path = fs::canonical(path, ec);
				if (!ec) {
					repo = Uri::canonical(path.string(), Uri("file://localhost"));
					if (verbose)
						printf("-- make FILE URL %s from %s\n", repo.string().c_str(), href.c_str());
				}
			}
		}

		if (repo.relative()) {
			// still relative -> prepend "http://"
			repo = "http://" + href;
			if (verbose)
				printf("-- make HTTP URL %s from %s\n", repo.string().c_str(), href.c_str());
		}
	}

	repo = Uri::normal(repo);
	if (repo.string().length()
		&& repo.string().at(repo.string().length() - 1) != '/') {
		repo = repo.string() + "/";
	}

	auto tr = db.transaction();
	tr.begin();

	if (db.add_repo(name, repo.string())) {
		if (verbose)
			printf("-- successfully added\n");
		tr.commit();
		return 0;
	}

	std::string orig_href;
	if (!db.repo_href(name, orig_href))
		parser.error("could not add " + href, true);

	auto orig = Uri::normal(orig_href);

	if (orig.string() == repo.string()) {
		if (verbose)
			printf("-- pair already in config\n");
		return 0;
	}

	parser.error("repo named `" + name + "` already points at " + orig.string() + ";\nif you want to use it with " + repo.string() + ", first remove the repo from config and then re-add it", true);

	return 0;
}

} // namespace add
} // namespace remote
