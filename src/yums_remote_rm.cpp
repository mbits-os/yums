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
namespace rm {

int call(args::parser& parser)
{
	bool verbose = false;
	std::string name;
	parser.set<std::true_type>(verbose, "v").help("show more output").opt();
	parser.positional(name).meta("NAME").help("the nickname of the repo to remove").req();
	parser.parse();

	if (name.empty())
		parser.error("argument NAME is required");

	yums_db db;
	if (!db.open_if_exists())
		parser.error("directory is not initialized", true);

	auto tr = db.transaction();
	tr.begin();

	std::string orig_href;
	if (!db.repo_href(name, orig_href))
		parser.error("no repo named `" + name + "` in config", true);

	if (db.rm_repo(name)) {
		if (verbose)
			printf("-- successfully removed\n");
		tr.commit();
		return 0;
	}

	parser.error("could not remove repo `" + name + "`", true);

	return 0;
}

} // namespace rm
} // namespace remote
