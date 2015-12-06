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

#pragma once

#include <data/dbconn.hpp>

struct yums_repo {
	long long id = 0;
	std::string name;
	std::string revision;
	std::string href;
};

class yums_db : public db::database_helper {
	int m_previous_version = -1;
public:
	enum { 
		initial_version = 1,
		latest_version = initial_version
	};

	static const char * const filename;

	yums_db();
	bool upgrade_schema(int current_version, int new_version);
	int previous_version();
	bool open_if_exists();
	db::transaction transaction() const { return db(); }

	bool add_repo(const std::string& name, const std::string& url);
	bool rm_repo(const std::string& name);
	bool repo_href(const std::string& name, std::string& url);
	bool repos(std::vector<yums_repo>& repos);
	bool update(const yums_repo& repo, std::string& error);
};