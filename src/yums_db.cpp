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

#include "yums_db.hpp"
#include "filesystem.hpp"
#include "repository.hpp"

const char * const yums_db::filename = ".yumsdb.sqlite";

yums_db::yums_db() : db::database_helper(filename, latest_version)
{
}

#define SQL(code) do { if (!conn->exec(code)) return false; } while(0)
bool yums_db::upgrade_schema(int current_version, int /*new_version*/)
{
	m_previous_version = current_version;
	if (current_version < initial_version) {
		auto conn = db();
		SQL("CREATE TABLE repo ("
			"id INTEGER PRIMARY KEY,"
			"name TEXT UNIQUE,"
			"revision TEXT,"
			"href TEXT UNIQUE"
			")");
		SQL("CREATE TABLE datafile ("
			"repo_id INTEGER,"
			"type TEXT,"
			"checksum TEXT,"
			"open_checksum TEXT,"
			"PRIMARY KEY (repo_id, type),"
			"FOREIGN KEY (repo_id) REFERENCES repo (id) ON DELETE CASCADE"
			")");
		SQL("CREATE TABLE package ("
			"id INTEGER PRIMARY KEY,"
			"repo_id INTEGER,"
			"pkgId TEXT NOT NULL,"
			"name TEXT NOT NULL,"
			"arch TEXT NOT NULL,"
			"epoch TEXT,"
			"version TEXT,"
			"release TEXT,"
			"summary TEXT,"
			"description TEXT,"
			"url TEXT,"
			"rpm_license TEXT,"
			"rpm_vendor TEXT,"
			"rpm_group TEXT,"
			"rpm_packager TEXT, "
			"location_href TEXT,"
			"checksum_type TEXT,"
			"FOREIGN KEY (repo_id) REFERENCES repo (id) ON DELETE CASCADE"
			")");
		SQL("CREATE TABLE requires ("
			"package_id INTEGER,"
			"name TEXT,"
			"flags TEXT,"
			"epoch TEXT,"
			"version TEXT,"
			"release TEXT,"
			"FOREIGN KEY (package_id) REFERENCES package (id) ON DELETE CASCADE"
			")");
		SQL("CREATE TABLE filelist ("
			"package_id INTEGER,"
			"filename TEXT,"
			"is_file INTEGER,"
			"FOREIGN KEY (package_id) REFERENCES package (id) ON DELETE CASCADE"
			")");
	}
	return true;
}
#undef SQL

int yums_db::previous_version()
{
	if (m_previous_version < 0) {
		// didn't upgrade
		m_previous_version = db()->version();
	}
	return m_previous_version;
}

bool yums_db::open_if_exists()
{
	fs::error_code ec;
	auto st = fs::status(yums_db::filename, ec);
	if (ec || !fs::exists(st))
		return false;

	if (!fs::is_regular_file(st) && !fs::is_symlink(st))
		return false;

	return open();
}

bool yums_db::add_repo(const std::string& name, const std::string& url)
{
	auto conn = db();
	auto stmt = conn->prepare("INSERT INTO repo (name, href) VALUES (?, ?)");
	stmt->bind(0, name.c_str());
	stmt->bind(1, url.c_str());
	return stmt->execute();
}

bool yums_db::rm_repo(const std::string& name)
{
	auto conn = db();
	auto stmt = conn->prepare("DELETE FROM repo WHERE name=?");
	stmt->bind(0, name.c_str());
	return stmt->execute();
}

bool yums_db::repo_href(const std::string& name, std::string& url)
{
	auto conn = db();
	auto stmt = conn->prepare("SELECT href FROM repo WHERE name=?");
	stmt->bind(0, name.c_str());
	auto cur = stmt->query();
	if (!cur || !cur->next())
		return false;

	url = cur->getString(0);
	return true;
}

namespace db {
	CURSOR_RULE(yums_repo)
	{
		CURSOR_ADD(0, id);
		CURSOR_ADD(1, name);
		CURSOR_ADD(2, revision);
		CURSOR_ADD(3, href);
	};
}

bool yums_db::repos(std::vector<yums_repo>& repos)
{
	auto conn = db();

	auto stmt = conn->prepare("SELECT id, name, revision, href FROM repo");
	auto cur = stmt->query();
	if (!cur) {
		return false;
	}

	repos.clear();
	return db::get(cur, repos);
}

bool yums_db::update(const yums_repo& repo, std::string& reason)
{
	using namespace repo;
	remote_repo remote { repo.href };
	error err = error::none;
	auto def = remote.read_index(err);
	if (err != error::none) {
		switch (err) {
		case error::no_repomd:
			reason = "cannot retrieve repository metadata (repomd.xml) for repository: " + repo.href + ". Please verify its path and try again.";
			break;
		case error::not_xml:
			reason = "cannot parse repository metadata (repomd.xml) for repository: " + repo.href + "..";
			break;
		default:
			break;
		}
		return false;
	}

	auto conn = db();
	auto tr = db::transaction { conn };
	if (!tr.begin()) {
		reason = "could not enter into transaction with config";
		return false;
	}

	auto stmt = conn->prepare("UPDATE repo SET revision=? WHERE id=?");
	stmt->bind(0, def.revision);
	stmt->bind(1, repo.id);
	if (!stmt->execute())
		return false;

	stmt = conn->prepare("INSERT OR REPLACE INTO datafile (repo_id, type, checksum, open_checksum) VALUES (?, ?, ?, ?)");
	stmt->bind(0, repo.id);
	stmt->bind(1, "primary");
	stmt->bind(2, def.primary.chksm.value);
	stmt->bind(3, def.primary.open_chksm.value);
	if (!stmt->execute())
		return false;

	stmt = conn->prepare("INSERT OR REPLACE INTO datafile (repo_id, type, checksum, open_checksum) VALUES (?, ?, ?, ?)");
	stmt->bind(0, repo.id);
	stmt->bind(1, "filelists");
	stmt->bind(2, def.filelists.chksm.value);
	stmt->bind(3, def.filelists.open_chksm.value);
	if (!stmt->execute())
		return false;

	// TODO: download and read primary.location and filelists.location

	tr.commit();
	return true;
}
