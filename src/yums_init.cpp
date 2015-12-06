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

#include <string>
using namespace std::literals;

namespace init {

int call(args::parser& parser)
{
	bool verbose = false;
	parser.set<std::true_type>(verbose, "v").help("show more output").opt();
	parser.parse();

	if (verbose)
		printf("-- check if already initialized\n");

	fs::error_code ec;
	auto cwd = fs::current_path(ec);
	if (ec)
		parser.error("fatal error, do not know where I am", true);


	auto st = fs::status(yums_db::filename, ec);
	if (!ec && fs::exists(st)) {
		if (!fs::is_regular_file(st) && !fs::is_symlink(st))
			parser.error(yums_db::filename + " must be a file"s, true);

		if (verbose)
			printf("-- check if %s is a yums config - upgrade if necessary\n", yums_db::filename);

		yums_db db;
		if (!db.open())
			parser.error("could not update yums config in " + cwd.string() + ".", true);

		if (db.previous_version() < db.latest_version)
			printf("updated from %d to %d.\n", db.previous_version(), db.latest_version);
		else
			printf("already initialized.\n");

		return 0;
	}

	if (verbose)
		printf("-- check if directory is empty\n");

	bool empty = true;
	for (auto& entry : fs::directory_iterator { cwd }) {
		empty = false;
		break;
	}

	if (!empty)
		parser.error("directory " + cwd.string() + " must be empty.", true);

	if (verbose)
		printf("-- write empty config\n");

	yums_db db;
	if (!db.open()) {
		fs::remove(yums_db::filename, ec); // ignore the ec here...
		parser.error("could not create yums config in " + cwd.string() + ".", true);
	}

	printf("initialized %s for yums.\n", cwd.string().c_str());
	return 0;
}

}
