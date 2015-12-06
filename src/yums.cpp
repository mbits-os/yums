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

#include "version.h"
#include <http/xhr.hpp>
#include "filesystem.hpp"

#if defined(_WIN32) && defined(_UNICODE)
using XChar = wchar_t;
#define main wmain
#else
using XChar = char;
#endif

struct command {
	const char* name;
	const char* description;
	int(*call)(args::parser&);
};

namespace init { int call(args::parser&); }
namespace remote { int call(args::parser&); }
namespace update { int call(args::parser&); }

command commands[] = {
	{ "init",  "Initializes empty directory for yums.", init::call },
	{ "remote",  "Manipulates the list of known remote repositories.", remote::call },
	{ "update",  "Updates config from repositories.", update::call },
};

namespace http = net::http::client;
using namespace std::literals;

class pushd {
	fs::path cwd_;
public:
	explicit pushd(fs::path&& cwd) : cwd_ { std::move(cwd) }
	{
	}
	~pushd()
	{
		fs::error_code ignore;
		fs::current_path(cwd_, ignore);
		if (ignore)
			fprintf(stderr, "could not return to %s\n", cwd_.string().c_str());
	}
};

int main(int argc, XChar* argv[])
{
	http::set_program_client_info(PROGRAM_NAME "/" PROGRAM_VERSION_STRING);

	std::string cwd;
	bool show_commands = false;
	args::parser base { { }, argc, argv };
	base.usage("[-h] [-C <dir>] [-a] <command> [<args>]");
	base.arg(cwd, "C").meta("<dir>").help("run as if yums was started in <dir> instead of current directory").opt();
	base.set<std::true_type>(show_commands, "a").help("show all recognized commands").opt();
	base.parse(args::with_subcommands);

	if (show_commands) {
		base.usage("[-C <dir>] <command> [<args>]");
		base.short_help();
		printf("\nknown commands:\n\n");

		std::vector<std::pair<std::string, std::string>> info;

		for (auto& cmd : commands)
			info.push_back(std::make_pair(cmd.name, cmd.description));

		base.format_list(info);
		return 0;
	}

	if (base.args().empty())
		base.error("command missing");

	fs::error_code ec;
	auto dir = fs::current_path(ec);
	if (ec)
		base.error("cannot save current directory");
	pushd save { std::move(dir) };

	if (!cwd.empty()) {
		fs::current_path(cwd, ec);
		if (ec)
			base.error("cannot change directory to " + cwd + ", error " + std::to_string(ec.value()) + ": " + ec.message(), true);
	}

	auto& name = base.args().front();
	for (auto& cmd : commands) {
		if (cmd.name != name)
			continue;

		args::parser sub { cmd.description, base.args() };
		sub.program(base.program() + " " + sub.program());

		return cmd.call(sub);
	}

	base.error("unknown command: " + name);
	return 0;
}
