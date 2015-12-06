/*
 * Copyright (C) 2013 midnightBITS
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

#include <dom/dom.hpp>
#include <dom/dom_xpath.hpp>
#include <dom/parsers/xml.hpp>
#include <vector>
#include <iterator>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#endif

namespace dom {

#if 0
	DocumentPtr Document::fromFile(const filesystem::path& path)
	{
		auto parser = parsers::xml::create(std::string());
		if (parser)
		{
			auto doc = parsers::parseFile(parser, path);
			if (doc)
				return doc;
		}

		parser = parsers::html::create(std::string());
		if (parser)
		{
			auto doc = parsers::parseFile(parser, path);
			if (doc)
				return doc;
		}

		return nullptr;
	}
#endif

	void Print(const NodeListPtr& subs, bool ignorews, size_t depth)
	{
		if (subs)
		{
			size_t count = subs->length();
			for(size_t i = 0; i < count; ++i)
				Print(subs->item(i), ignorews, depth);
		}
	}

	template <typename T>
	inline std::string qName(std::shared_ptr<T> ptr)
	{
		QName qname = ptr->nodeQName();
		if (qname.nsName.empty()) return qname.localName;
		return "{" + qname.nsName + "}" + qname.localName;
	}

	void Print(const NodePtr& node, bool ignorews, size_t depth)
	{
		NodeListPtr subs = node->childNodes();

		NODE_TYPE type = node->nodeType();
		std::string out;
		for (size_t i = 0; i < depth; ++i) out += "    ";

		if (type == TEXT_NODE)
		{
			std::string val = node->nodeValue();
			if (ignorews)
			{
				size_t lo = 0, hi = val.length();
				while (lo < hi && val[lo] && isspace((unsigned char)val[lo])) lo++;
				while (lo < hi && isspace((unsigned char)val[hi-1])) hi--;
				val = val.substr(lo, hi - lo);
				if (val.empty()) return;
			}
			if (val.length() > 80)
				val = val.substr(0, 77) + "[...]";
			out += "# " + val;
		}
		else if (type == ELEMENT_NODE)
		{
			std::string sattrs;
			auto e = std::static_pointer_cast<Element>(node);
			NodeListPtr attrs;
			if (e) attrs = e->getAttributes();
			if (attrs)
			{
				size_t count = attrs->length();
				for(size_t i = 0; i < count; ++i)
				{
					NodePtr node = attrs->item(i);
					sattrs += " " + qName(node) + "='" + node->nodeValue() + "'";
				}
			}

			if (subs && subs->length() == 1)
			{
				NodePtr sub = subs->item(0);
				if (sub && sub->nodeType() == TEXT_NODE)
				{
					out += qName(node);
					if (!sattrs.empty())
						out += "[" + sattrs + " ]";

					std::string val = sub->nodeValue();
					if (ignorews)
					{
						size_t lo = 0, hi = val.length();
						while (val[lo] && isspace((unsigned char)val[lo])) lo++;
						while (lo < hi && isspace((unsigned char)val[hi-1])) hi--;
						val = val.substr(lo, hi - lo);
						if (val.empty()) return;
					}

					if (val.length() > 80)
						val = val.substr(0, 77) + "[...]";

					out += ": " + val + "\n";
					fprintf(stderr, "%s", out.c_str());
#ifdef WIN32
					OutputDebugStringA(out.c_str());
#endif
					return;
				}
			}
			if (!subs || subs->length() == 0)
			{
					out += qName(node);
					if (!sattrs.empty())
						out += "[" + sattrs + " ]";
					out += "\n";
					fprintf(stderr, "%s", out.c_str());
#ifdef WIN32
					OutputDebugStringA(out.c_str());
#endif
					return;
			}
			out += "<" + qName(node) + sattrs + ">";
		}
		out += "\n";
		fprintf(stderr, "%s", out.c_str());
#ifdef WIN32
		OutputDebugStringA(out.c_str());
#endif

		Print(subs, ignorews, depth+1);
	}
}
