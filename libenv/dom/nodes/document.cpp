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

#include "element.hpp"
#include "document.hpp"
#include "attribute.hpp"
#include "text.hpp"
#include "document_fragment.hpp"

namespace dom { namespace impl {

	Document::Document()
	{
		m_qname.localName = "#document";
	}

	NodeListPtr Document::childNodes()
	{
		if (fragment)
			return fragment->childNodes();

		if (!root)
			return nullptr;

		try {
			NodePtrs children;
			children.push_back(root);
			return std::make_shared<NodeList>(children);
		}
		catch (std::bad_alloc) { return nullptr; }
	}

	void Document::setDocumentElement(const dom::ElementPtr& elem)
	{
		root = elem;
		fragment = nullptr;
		if (elem)
			((NodeImplInit*)elem->internalData())->fixQName();
	}

	void Document::setFragment(const DocumentFragmentPtr& f)
	{
		root = nullptr;
		fragment = f;
		if (f)
			((NodeImplInit*)f->internalData())->fixQName();
	}
	dom::ElementPtr Document::createElement(const std::string& tagName)
	{
		NodeImplInit init;
		init.type = ELEMENT_NODE;
		init._name = tagName;
		//init._value;
		init.document = shared_from_this();
		init.index = 0;
		return std::make_shared<Element>(init);
	}

	dom::TextPtr Document::createTextNode(const std::string& data)
	{
		NodeImplInit init;
		init.type = TEXT_NODE;
		//init._name;
		init._value = data;
		init.document = shared_from_this();
		init.index = 0;
		return std::make_shared<Text>(init);
	}

	dom::AttributePtr Document::createAttribute(const std::string& name, const std::string& value)
	{
		NodeImplInit init;
		init.type = ATTRIBUTE_NODE;
		init._name = name;
		init._value = value;
		init.document = shared_from_this();
		init.index = 0;
		return std::make_shared<Attribute>(init);
	}

	dom::DocumentFragmentPtr Document::createDocumentFragment()
	{
		NodeImplInit init;
		init.type = DOCUMENT_FRAGMENT_NODE;
		init._name = "#document-fragment";
		init.document = shared_from_this();
		init.index = 0;
		return std::make_shared<DocumentFragment>(init);
	}

	dom::NodeListPtr Document::getElementsByTagName(const std::string& tagName)
	{
		if (root)
			return root->getElementsByTagName(tagName);
		if (fragment)
			return fragment->getElementsByTagName(tagName);
		return nullptr;
	}

	dom::ElementPtr Document::getElementById(const std::string& elementId)
	{
		return nullptr;
	}

	NodePtr Document::find(const std::string& path, const Namespaces& ns)
	{
		return xpath::XPath(path, ns).find(shared_from_this());
	}

	NodeListPtr Document::findall(const std::string& path, const Namespaces& ns)
	{
		return xpath::XPath(path, ns).findall(shared_from_this());
	}
}}

namespace dom {
	DocumentPtr Document::create()
	{
		return std::make_shared<impl::Document>();
	}
}
