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

#include "repository.hpp"
#include "filesystem.hpp"
#include "http/xhr.hpp"
#include <dom/parsers/xml.hpp>
#include <dom/dom.hpp>
#include <cassert>
#include <random>

namespace repo {

namespace http = net::http::client;

template <typename Pred>
error remote_repo::http_get(const char* uri, Pred&& pred) const
{
	auto loader = http::create();
	error err = error::none;

	loader->onreadystatechange([&](http::XmlHttpRequest* xhr) {
		if (xhr->getReadyState() == http::XmlHttpRequest::HEADERS_RECEIVED) {
			if (xhr->getStatus() / 100 != 2) {
				xhr->abort();
				err = error::got_404;
				return;
			}
		}

		if (xhr->getReadyState() == http::XmlHttpRequest::DONE) {
			err = pred(xhr);
		}
	});

	std::fprintf(stderr, "OPEN %s\n", Uri::canonical(uri, m_root).string().c_str());
	loader->open(http::HTTP_GET, Uri::canonical(uri, m_root).string(), false);
	loader->send();

	return err;
}

static dom::NSData namespaces[] = {
	{ "repo", "http://linux.duke.edu/metadata/repo" },
	{ nullptr, nullptr }
};

dom::ElementPtr element_cast(const dom::NodePtr& node)
{
	if (node && node->nodeType() == dom::ELEMENT_NODE)
		return std::static_pointer_cast<dom::Element>(node);
	return { };
}

repomd remote_repo::read_index(error& err) const
{
	repomd out;
	err = http_get("repodata/repomd.xml", [&] (http::XmlHttpRequest* xhr) {
		auto doc = dom::parsers::xml::parseDocument({ }, xhr->getResponseText(), xhr->getResponseTextLength());
		if (!doc)
			return error::not_xml;

		auto rev = doc->find("/repo:repomd/repo:revision", namespaces);
		if (rev)
			out.revision = rev->stringValue();

		auto data = doc->findall("/repo:repomd/repo:data", namespaces);
		if (data) {
			auto len = data->length();
			for (size_t i = 0; i < len; ++i) {
				auto item = data->element(i);
				assert(item);

				if (!item->hasAttribute("type"))
					continue;

				auto type = item->getAttribute("type");
				if (type != "primary" && type != "filelists")
					continue;

				auto& out_item = type == "primary" ? out.primary : out.filelists;

				auto child = item->find("repo:location/@href", namespaces);
				if (!child)
					continue;

				out_item.location = child->stringValue();

				auto sub = element_cast(item->find("repo:checksum", namespaces));
				if (sub && sub->hasAttribute("type")) {
					out_item.chksm.type = sub->getAttribute("type");
					out_item.chksm.value = sub->stringValue();
				}

				sub = element_cast(item->find("repo:open-checksum", namespaces));
				if (sub && sub->hasAttribute("type")) {
					out_item.open_chksm.type = sub->getAttribute("type");
					out_item.open_chksm.value = sub->stringValue();
				}
			}
		}

		return error::none;
	});

	if (err == error::got_404)
		err = error::no_repomd;

	return out;
}

static uint8_t rand_hex()
{
	static std::mt19937 mt { std::random_device{}() };
	static std::uniform_int_distribution<> hex { 0, 15 };
	return hex(mt);
}

fs::path unique_path(fs::error_code& ec)
{
	static constexpr char alphabet[] = "0123456789abcdef";

	std::string out { "repo-%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%.xml.gz" };
	for (auto& c : out) {
		if (c == '%')
			c = alphabet[rand_hex()];
	}
	return out;
}

std::string remote_repo::get_datafile(const data& file, error& err) const
{
	fs::error_code ec;
	auto temp = fs::temp_directory_path(ec);
	if (ec) {
		err = error::no_temp_dir;
		return { };
	}

	while (true) {
		auto dest = temp / unique_path(ec);
		printf("? %s...\n", dest.string().c_str());

		if (fs::exists(dest, ec))
			continue;
		if (ec)
			break;

		printf("?     fopen\n");
		std::unique_ptr<FILE, decltype(&fclose)> dst_ptr { nullptr, fclose };
		dst_ptr.reset(fs::fopen(dest, "wb"));
		if (!dst_ptr)
			break;

		auto dst = dst_ptr.get();
		printf("?     HTTP-GET\n");
		err = http_get(file.location.c_str(), [dst](http::XmlHttpRequest* xhr) {
			auto ptr = xhr->getResponseText();
			auto size = xhr->getResponseTextLength();
			printf("?     Got %zu bytes @%p\n", size, ptr);
			while (size) {
				auto part = fwrite(ptr, 1, size, dst);
				printf("?     Wrote %zu\n", part);
				if (!part)
					break;
				size -= part;
			}
			return error::none;
		});
		return dest.string();
	}

	err = error::no_temp_file;
	return { };
}

}
