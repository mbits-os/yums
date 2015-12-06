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
#include <string.h>

namespace dom { namespace impl {

	Element::Element(const Init& init) : ParentNodeImpl(init), nsRebuilt(false) {}

	std::string Element::getAttribute(const std::string& name)
	{
		std::map< std::string, dom::AttributePtr >::const_iterator
			_it = lookup.find(name);
		if (_it == lookup.end()) return std::string();
		return _it->second->value();
	}

	dom::AttributePtr Element::getAttributeNode(const std::string& name)
	{
		std::map< std::string, dom::AttributePtr >::const_iterator
			_it = lookup.find(name);
		if (_it == lookup.end()) return dom::AttributePtr();
		return _it->second;
	}

	bool Element::setAttribute(const dom::AttributePtr& attr)
	{
		NodeImplInit* p = (NodeImplInit*)attr->internalData();
		if (p)
			p->parent = shared_from_this();

		std::map< std::string, dom::AttributePtr >::const_iterator
			_it = lookup.find(attr->name());
		if (_it != lookup.end())
		{
			_it->second->value(attr->value());
			return true;
		}
		lookup[attr->name()] = attr;
		return true;
	}

	bool Element::removeAttribute(const AttributePtr& attr)
	{
		if (!attr)
			return false;
		return Element::removeAttribute(attr->name());
	}

	bool Element::setAttribute(const std::string& attr, const std::string& value)
	{
		auto doc = Element::ownerDocument();
		if (!doc)
			return false;
		auto attribute = doc->createAttribute(attr, value);
		if (!attribute)
			return false;

		return Element::setAttribute(attribute);
	}

	bool Element::removeAttribute(const std::string& attr)
	{
		auto it = lookup.find(attr);
		if (it == lookup.end())
			return false;
		lookup.erase(it);
		return true;
	}

	dom::NodeListPtr Element::getAttributes()
	{
		NodePtrs out;
		out.reserve(lookup.size());
		for (auto& pair : lookup)
			out.push_back(pair.second);
		return std::make_shared<NodeList>(out);
	}

	bool Element::hasAttribute(const std::string& name)
	{
		std::map< std::string, dom::AttributePtr >::const_iterator
			_it = lookup.find(name);
		return _it != lookup.end();
	}

	void Element::enumTagNames(const std::string& tagName, NodePtrs& out)
	{
		if (tagName == _name) out.push_back(shared_from_this());

		for (auto&& node : children)
		{
			if (node->nodeType() == ELEMENT_NODE)
				((Element*)node.get())->enumTagNames(tagName, out);
		}
	}

	dom::NodeListPtr Element::getElementsByTagName(const std::string& tagName)
	{
		NodePtrs out;
		enumTagNames(tagName, out);
		return std::make_shared<NodeList>(out);
	}

	bool Element::appendAttr(const dom::NodePtr& newChild)
	{
		if (!newChild || newChild->nodeType() != dom::ATTRIBUTE_NODE)
			return false;
		return setAttribute(std::static_pointer_cast<dom::Attribute>(newChild));
	}
	bool Element::removeAttr(const dom::NodePtr& child)
	{
		if (!child || child->nodeType() != dom::ATTRIBUTE_NODE)
			return false;
		return removeAttribute(std::static_pointer_cast<dom::Attribute>(child));
	}

	std::string Element::innerText()
	{
		//special case:
		if (children.size() == 1 && children[0] && children[0]->nodeType() == TEXT_NODE)
			return children[0]->nodeValue();

		std::string out;

		for (auto&& node : children)
		{
			if (node) {
				switch (node->nodeType()) {
				case TEXT_NODE: out += node->nodeValue(); break;
				case ELEMENT_NODE:
					out += std::static_pointer_cast<dom::Element>(node)->innerText();
					break;
				default:
					break;
				};
			}
		}

		return out;
	}

	void Element::fixQName(bool forElem)
	{
		NodeImpl<Element, dom::Element>::fixQName(forElem);
		if (!forElem) return;
		for (auto&& pair : lookup)
		{
			if (strncmp(pair.first.c_str(), "xmlns", 5) == 0 &&
				(pair.first.length() == 5 || pair.first[5] == ':'))
			{
				continue;
			}
			NodeImplInit* p = (NodeImplInit*)pair.second->internalData();
			if (!p) continue;
			p->fixQName(false);
		};
	}

	void Element::fixQName(QName& qname, const std::string& ns, const std::string& localName)
	{
		if (!nsRebuilt)
		{
			nsRebuilt = true;
			namespaces.clear();
			for (auto&& pair : lookup)
			{
				if (strncmp(pair.first.c_str(), "xmlns", 5) != 0) continue;
				if (pair.first.length() != 5 && pair.first[5] != ':') continue;
				if (pair.first.length() == 5) namespaces[""] = pair.second->value();
				else namespaces[std::string(pair.first.c_str() + 6)] = pair.second->value();
			};
		}
		InternalNamespaces::const_iterator _it = namespaces.find(ns);
		if (_it != namespaces.end())
		{
			qname.nsName = _it->second;
			qname.localName = localName;
			return;
		}
		NodeImpl<Element, dom::Element>::fixQName(qname, ns, localName);
	}
}}
