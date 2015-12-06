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

#ifndef __DOM_INTERNAL_ELEMENT_HPP__
#define __DOM_INTERNAL_ELEMENT_HPP__

#include "parent_node_impl.hpp"

namespace dom { namespace impl {

	class Element : public ParentNodeImpl<Element, dom::Element>
	{
		typedef std::map< std::string, std::string > InternalNamespaces;
		InternalNamespaces namespaces;
		std::map< std::string, dom::AttributePtr > lookup;
		bool nsRebuilt;
	public:
		Element(const Init& init);

		std::string getAttribute(const std::string& name) override;
		dom::AttributePtr getAttributeNode(const std::string& name) override;
		bool setAttribute(const dom::AttributePtr& attr) override;
		bool removeAttribute(const AttributePtr& attr) override;
		bool setAttribute(const std::string& attr, const std::string& value) override;
		bool removeAttribute(const std::string& attr) override;
		dom::NodeListPtr getAttributes() override;
		bool hasAttribute(const std::string& name) override;
		void enumTagNames(const std::string& tagName, NodePtrs& out);
		dom::NodeListPtr getElementsByTagName(const std::string& tagName) override;
		bool appendAttr(const dom::NodePtr& newChild);
		bool removeAttr(const dom::NodePtr& child);
		std::string innerText() override;
		void fixQName(bool forElem = true) override;
		void fixQName(QName& qname, const std::string& ns, const std::string& localName) override;
	};
}}

#endif // __DOM_INTERNAL_ELEMENT_HPP__