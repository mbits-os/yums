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

#include <dom/parsers/xml.hpp>
#include <dom/dom.hpp>
#include "expat.hpp"

namespace dom { namespace parsers { namespace xml {

	class Parser : public parsers::Parser, public ::xml::ExpatBase<Parser>
	{
		dom::ElementPtr elem;
		std::string text;
		dom::DocumentPtr doc;

		void addText()
		{
			if (text.empty()) return;
			if (elem)
				elem->appendChild(doc->createTextNode(text));
			text.clear();
		}
	public:

		Parser() : doc(dom::Document::create()) {}

		bool create(const std::string& cp)
		{
			if (!doc)
				return false;
			return ::xml::ExpatBase<Parser>::create(cp.empty() ? nullptr : cp.c_str());
		}

		bool supportsChunks() const override { return true; }
		bool onData(const void* data, size_t length) override
		{
			return parse((const char*)data, length, false);
		}
		DocumentPtr onFinish() override
		{
			if (!parse(nullptr, 0))
				return nullptr;

			return doc;
		}

		bool onUnknownEncoding(const XML_Char* name, XML_Encoding* info)
		{
			info->data = nullptr;
			info->convert = nullptr;
			info->release = nullptr;

			return false;
		}

		void onStartElement(const XML_Char *name, const XML_Char **attrs)
		{
			addText();
			auto current = doc->createElement(name);
			if (!current) return;
			for (; *attrs; attrs += 2)
			{
				auto attr = doc->createAttribute(attrs[0], attrs[1]);
				if (!attr) continue;
				current->setAttribute(attr);
			}
			if (elem)
				elem->appendChild(current);
			else
				doc->setDocumentElement(current);
			elem = current;
		}

		void onEndElement(const XML_Char *name)
		{
			addText();
			if (!elem) return;
			dom::NodePtr node = elem->parentNode();
			elem = std::static_pointer_cast<dom::Element>(node);
		}

		void onCharacterData(const XML_Char *pszData, int nLength)
		{
			text += std::string(pszData, nLength);
		}
	};

	ParserPtr create(const std::string& encoding)
	{
		try
		{
			auto parser = std::make_shared<Parser>();
			if (!parser->create(encoding))
				return nullptr;

			parser->enableElementHandler();
			parser->enableCharacterDataHandler();
			parser->enableUnknownEncodingHandler();

			return parser;
		}
		catch (std::bad_alloc&)
		{
			return nullptr;
		}
	}

}}}
