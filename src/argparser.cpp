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

#include <argparser.hpp>

#include <cstdlib>
#include <deque>

void args::parser::short_help(FILE* out)
{
	fprintf(out, "usage: %s", prog_.c_str());

	if (!usage_.empty()) {
		fprintf(out, " %s\n", usage_.c_str());
		return;
	}

	fprintf(out, " [-h]");
	for (auto& action : actions_) {
		if (action->is_positional()) {
			auto meta = action->meta().empty() ? "N" : action->meta();
			if (action->required()) {
				fprintf(out, " %s", meta.c_str());
				if (action->is_repeating())
					fprintf(out, " [%s ...]", meta.c_str());
			} else {
				fprintf(out, " [%s%s]", meta.c_str(),
					action->is_repeating() ? " ..." : "");
			}
			continue;
		}

		if (action->names().empty())
			continue;

		fprintf(out, " ");
		if (!action->required())
			fprintf(out, "[");
		auto& name = action->names().front();
		if (name.length() == 1)
			fprintf(out, "-%s", name.c_str());
		else
			fprintf(out, "--%s", name.c_str());
		if (action->needs_arg()) {
			if (action->meta().empty())
				fprintf(out, " ARG");
			else {
				fprintf(out, " %s", action->meta().c_str());
			}
		}
		if (!action->required())
			fprintf(out, "]");
	}

	fprintf(out, "\n");
}

void args::parser::help()
{
	short_help();

	if (!description_.empty())
		printf("\n%s\n", description_.c_str());

	printf("\narguments:\n\n");
	std::vector<std::pair<std::string, std::string>> info;
	info.reserve(1 + actions_.size());
	info.push_back(std::make_pair("-h, --help", "show this help message and exit"));
	for (auto& action : actions_) {
		if (action->is_positional())
			continue;

		std::string names;
		bool first = true;
		for (auto& name : action->names()) {
			if (first) first = false;
			else names.append(", ");

			if (name.length() == 1)
				names.append("-");
			else
				names.append("--");
			names.append(name);
		}
		if (names.empty())
			continue;

		if (action->needs_arg()) {
			std::string arg;
			if (action->meta().empty())
				arg = " ARG";
			else
				arg = " " + action->meta();
			names.append(arg);
		}

		info.push_back(std::make_pair(names, action->help()));
	}

	for (auto& action : actions_) {
		if (!action->is_positional())
			continue;

		auto name = action->meta().empty() ? "N" : action->meta();
		info.push_back(std::make_pair(std::move(name), action->help()));
	}

	format_list(info);

	std::exit(0);
}

void args::parser::format_list(const std::vector<std::pair<std::string, std::string>>& info)
{
	size_t len = 0;
	for (auto& pair : info) {
		if (len < std::get<0>(pair).length())
			len = std::get<0>(pair).length();
	}

	for (auto& pair : info) {
		printf("%-*s %s\n", (int)len, std::get<0>(pair).c_str(), std::get<1>(pair).c_str());
	}
}

void args::parser::error(const std::string& msg, bool quick)
{
	fflush(stdout);
	fflush(stderr);
	if (!quick)
		short_help(stderr);
	fprintf(stderr, "%s: error: %s\n", prog_.c_str(), msg.c_str());
	std::exit(2);
}

void args::parser::program(const std::string& value)
{
	prog_ = value;
}

const std::string& args::parser::program()
{
	return prog_;
}

void args::parser::usage(const std::string& value)
{
	usage_ = value;
}

const std::string& args::parser::usage()
{
	return usage_;
}

void args::parser::parse(subcommands policy)
{
	std::deque<action*> positionals;

	for (auto& ptr : actions_) {
		if (ptr->is_positional())
			positionals.push_back(ptr.get());
	}

	auto count = args_.size();
	for (decltype(count) i = 0; i < count; ++i) {
		auto& arg = args_[i];

		if (arg.length() > 1 && arg[0] == '-') {
			bool should_continue;
			if (arg.length() > 2 && arg[1] == '-')
				should_continue = parse_long(arg.substr(2), i, policy);
			else
				should_continue = parse_short(arg.substr(1), i, policy);
			if (!should_continue) {
				count = i;
				break;
			}
		} else {
			if (!positionals.empty()) {
				auto action = positionals.front();
				action->visit(*this, arg);
				if (!action->is_repeating())
					positionals.pop_front();
			} else if (policy != take_all) {
				count = i;
				break;
			} else
				error("unrecognized argument: " + arg);
		}
	}

	for (auto& action : actions_) {
		if (action->required() && !action->visited()) {
			if (action->is_positional()) {
				auto& name = action->meta();

				auto arg = name.empty() ? "N" : name;
				error("argument " + arg + " is required");
			} else {
				auto& name = action->names().front();

				auto arg = name.length() == 1 ? "-" + name : "--" + name;
				error("argument " + arg + " is required");
			}
		}
	}

	if (policy == with_subcommands) {
		std::vector<std::string> copy { std::next(args_.begin(), count), args_.end() };
		args_.swap(copy);
	}
}

bool args::parser::parse_long(const std::string& name, size_t& i, subcommands policy) {
	if (name == "help")
		help();

	for (auto& action : actions_) {
		if (!action->is(name))
			continue;

		if (action->needs_arg()) {
			++i;
			if (i >= args_.size())
				error("argument --" + name + ": expected one argument");

			action->visit(*this, args_[i]);
		}
		else
			action->visit(*this);

		return true;
	}

	if (policy == with_subcommands)
		return false;

	error("unrecognized argument: --" + name);
}

static inline std::string expand(char c) {
	char buff[] = { c, 0 };
	return buff;
}

bool args::parser::parse_short(const std::string& name, size_t& arg, subcommands policy)
{
	auto length = name.length();
	for (decltype(length) i = 0; i < length; ++i) {
		auto c = name[i];
		if (c == 'h')
			help();

		bool found = false;
		for (auto& action : actions_) {
			if (!action->is(c))
				continue;

			if (action->needs_arg()) {
				std::string param;

				++i;
				if (i < length)
					param = name.substr(i);
				else {
					++arg;
					if (arg >= args_.size())
						error("argument -" + expand(c) + ": expected one argument");

					param = args_[arg];
				}

				i = length;

				action->visit(*this, param);
			}
			else
				action->visit(*this);

			found = true;
			break;
		}

		if (!found) {
			if (policy == with_subcommands)
				return false;

			error("unrecognized argument: -" + expand(c));
		}
	}

	return true;
}
