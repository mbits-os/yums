#pragma once

#include <http/uri.hpp>
#include <string>

namespace repo {

enum class data_type {
	unknown,
	deltainfo,
	filelists,
	other,
	primary,
	susedata,
	suseinfo,
	updateinfo,
	patches,
	products,
	product,
	patterns,
	pattern,
};

struct checksum {
	std::string type;
	std::string value;
};

struct data {
	data_type e_type;
	std::string location;
	checksum chksm;
	checksum open_chksm;
};

struct repomd {
	std::string revision;
	data primary;
	data filelists;
};

enum class error {
	none = 0,
	no_repomd,
	no_primary,
	no_filelists,
	got_404,
	not_xml,
	no_temp_dir,
	no_temp_file
};

class remote_repo {
	Uri m_root;

	template <typename Pred>
	error http_get(const char* uri, Pred&& pred) const;
public:
	explicit remote_repo(const Uri& root) : m_root(root)
	{
	}

	repomd read_index(error&) const;
	std::string get_datafile(const data&, error&) const;
};

}
