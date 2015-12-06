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
#include "yums_db.hpp"

#include <string>
using namespace std::literals;

namespace remote {
namespace show {

int call(args::parser& parser)
{
	bool verbose = false;
	std::string name;
	parser.positional(name).meta("NAME").help("show details about the named repository").opt();
	parser.parse();

	yums_db db;
	if (!db.open_if_exists())
		parser.error("directory is not initialized", true);

	if (name.empty()) {
		std::vector<yums_repo> repos;
		if (!db.repos(repos))
			parser.error("could not list repos", true);

		for (auto& repo : repos)
			printf("%s\n", repo.name.c_str());

		return 0;
	}

	std::string href;
	if (!db.repo_href(name, href))
		parser.error("no repo named `" + name + "` in config", true);

	printf("%s: %s\n", name.c_str(), href.c_str());

	return 0;
}

} // namespace show
} // namespace remote
