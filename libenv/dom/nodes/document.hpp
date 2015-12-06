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

#ifndef __DOM_INTERNAL_DOCUMENT_HPP__
#define __DOM_INTERNAL_DOCUMENT_HPP__

#include <dom/nodes/document.hpp>

namespace dom { namespace impl {

	class Document : public dom::Document, public std::enable_shared_from_this<Document>
	{
		QName m_qname;
		dom::ElementPtr root;
		dom::DocumentFragmentPtr fragment;
	public:
		Document();

		std::string nodeName() const override { return m_qname.localName; }
		const QName& nodeQName() const override { return m_qname; }
		std::string nodeValue() const override { return std::string(); }
		void nodeValue(const std::string&) override {}
		NODE_TYPE nodeType() const override { return DOCUMENT_NODE; }

		NodePtr parentNode() override { return nullptr; }
		NodePtr firstChild() override { return documentElement(); }
		NodePtr lastChild() override { return documentElement(); }
		NodePtr previousSibling() override { return nullptr; }
		NodePtr nextSibling() override { return nullptr; }
		NodeListPtr childNodes() override;
		DocumentPtr ownerDocument() override { return shared_from_this(); }
		bool insertBefore(const NodePtr& child, const NodePtr& before = nullptr) override { return false; }
		bool insertBefore(const NodeListPtr& children, const NodePtr& before = nullptr) override { return false; }
		bool appendChild(const NodePtr& newChild) override { return false; }
		bool replaceChild(const NodePtr& newChild, const NodePtr& oldChild) override { return false; }
		bool replaceChild(const NodeListPtr& newChildren, const NodePtr& oldChild) override { return false; }
		bool removeChild(const NodePtr& child) override { return false; }
		void* internalData() override { return nullptr; }

		dom::ElementPtr documentElement() override { return root; }
		void setDocumentElement(const dom::ElementPtr& elem) override;
		dom::DocumentFragmentPtr associatedFragment() override { return fragment; }
		void setFragment(const DocumentFragmentPtr& f) override;
		dom::ElementPtr createElement(const std::string& tagName) override;
		dom::TextPtr createTextNode(const std::string& data) override;
		dom::AttributePtr createAttribute(const std::string& name, const std::string& value) override;
		dom::DocumentFragmentPtr createDocumentFragment() override;
		dom::NodeListPtr getElementsByTagName(const std::string& tagName) override;
		dom::ElementPtr getElementById(const std::string& elementId) override;
		NodePtr find(const std::string& path, const Namespaces& ns) override;
		NodeListPtr findall(const std::string& path, const Namespaces& ns) override;
	};
}}

#endif // __DOM_INTERNAL_DOCUMENT_HPP__
