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

namespace remote {
struct command {
	const char* name;
	const char* description;
	int(*call)(args::parser&);
};

namespace add { int call(args::parser&); }
namespace rm { int call(args::parser&); }
namespace show { int call(args::parser&); }

command commands[] = {
	{ "add",  "Adds a repository to the yums config.", add::call },
	{ "rm",  "Removes a repository from the yums config.", rm::call },
	{ "show",  "Show info about known remotes.", show::call },
};

int call(args::parser& parser)
{
	bool show_commands = false;
	parser.usage("[-a] <command> [<args>]");
	parser.set<std::true_type>(show_commands, "a").help("show all recognized remote sub-commands").opt();
	parser.parse(args::with_subcommands);

	if (show_commands) {
		parser.usage("<command> [<args>]");
		parser.short_help();
		printf("\nknown commands:\n\n");

		std::vector<std::pair<std::string, std::string>> info;

		for (auto& cmd : commands)
			info.push_back(std::make_pair(cmd.name, cmd.description));

		parser.format_list(info);
		return 0;
	}

	if (parser.args().empty())
		parser.error("command missing");

	auto& name = parser.args().front();
	for (auto& cmd : commands) {
		if (cmd.name != name)
			continue;

		args::parser sub { cmd.description, parser.args() };
		sub.program(parser.program() + " " + sub.program());

		return cmd.call(sub);
	}

	parser.error("unknown command: " + name);
	return 0;
}

}
