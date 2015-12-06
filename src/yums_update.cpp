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
#include <algorithm>

#include <string>
using namespace std::literals;

namespace update {

int call(args::parser& parser)
{
	bool verbose = false;
	std::vector<std::string> names;
	parser.set<std::true_type>(verbose, "v").help("show more output").opt();
	parser.positional(names).meta("NAME").help("the names of the repos to update; if not present, will update all repos").opt();
	parser.parse();

	names.erase(
		std::remove_if(
			std::begin(names), std::end(names),
			[](auto& val) { return val.empty(); }
			),
		std::end(names)
		);

	yums_db db;
	if (!db.open_if_exists())
		parser.error("directory is not initialized", true);

	std::vector<yums_repo> repos;

	if (!db.repos(repos))
		parser.error("could not list repos", true);

	if (!names.empty()) {
		auto filter = [&](auto& repo) -> bool
		{
			auto it = std::begin(names);
			auto end = std::end(names);
			for (; it != end; ++it) {
				if (*it != repo.name)
					continue;
				names.erase(it);
				return false;
			}
			return true;
		};
		repos.erase(
			std::remove_if(std::begin(repos), std::end(repos), filter),
			std::end(repos)
			);
	}

	if (!names.empty())
		parser.error("no repository named `" + names.front() + "`", true);

	for (auto& repo : repos) {
		std::string error;
		if (!db.update(repo, error)) {
			if (error.empty())
				error = "reason unknown";
			parser.error("could not update `" + names.front() + "`: " + error, true);
		}
	}

	return 0;
}

}
