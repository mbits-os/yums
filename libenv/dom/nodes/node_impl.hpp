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

#ifndef __DOM_INTERNAL_NODE_IMPL_HPP__
#define __DOM_INTERNAL_NODE_IMPL_HPP__

#include <map>
#include <dom/dom.hpp>
#include <dom/dom_xpath.hpp>
#include "nodelist.hpp"

namespace dom { namespace impl {

	static inline NodePtr createTextSibling(Node* node, const std::string& data)
	{
		if (!node) return nullptr;
		auto doc = node->ownerDocument();
		if (!doc) return nullptr;
		return doc->createTextNode(data);
	}

	static inline bool removeFromParent(const NodePtr& node)
	{
		if (!node)
			return false;
		auto parent = node->parentNode();
		if (!parent)
			return true;
		return parent->removeChild(node);
	}

	struct NodeImplInit
	{
		NODE_TYPE type;
		std::string _name, _value;
		NodePtrs children;
		std::weak_ptr<dom::Document> document;
		std::weak_ptr<dom::Node> parent;
		size_t index = (size_t)-1;
		QName qname;

		virtual void fixQName(bool forElem = true)
		{
			std::string::size_type col = _name.find(':');
			if (col == std::string::npos && !forElem) return;
			if (col == std::string::npos)
				fixQName(qname, std::string(), _name);
			else
				fixQName(qname, std::string(_name.c_str(), col), std::string(_name.c_str() + col + 1));
		}

		virtual void fixQName(QName& qname, const std::string& ns, const std::string& localName)
		{
			dom::NodePtr parent = this->parent.lock();
			if (!parent) return;
			NodeImplInit* p = (NodeImplInit*)parent->internalData();
			if (!p) return;
			p->fixQName(qname, ns, localName);
		}
	};

	template <typename T, typename _Interface>
	class NodeImpl : public _Interface, public NodeImplInit, public std::enable_shared_from_this<T>
	{
	public:

		typedef NodeImplInit Init;
		typedef _Interface Interface;

		NodeImpl(const Init& init) : Init(init)
		{
			qname.localName = init._name;
		}

		std::string nodeName() const override { return _name; }
		const QName& nodeQName() const override { return qname; }
		std::string nodeValue() const override { return _value; }
		void nodeValue(const std::string& val) override
		{
			if (type != ELEMENT_NODE)
				_value = val;
		}

		NODE_TYPE nodeType() const override { return type; }

		dom::NodePtr parentNode() override { return parent.lock(); }
		dom::NodeListPtr childNodes() override
		{
			try {
				return std::make_shared<NodeList>(children);
			}
			catch (std::bad_alloc) { return nullptr; }
		}

		dom::NodePtr firstChild() override
		{
			if (!children.size()) return dom::NodePtr();
			return children[0];
		}

		dom::NodePtr lastChild() override
		{
			if (!children.size()) return dom::NodePtr();
			return children[children.size() - 1];
		}

		dom::NodePtr previousSibling() override
		{
			dom::NodePtr par = parent.lock();
			if (!index || !par)
				return dom::NodePtr();

			NodeImplInit* plist = (NodeImplInit*)par->internalData();
			if (!plist) return nullptr;
			return plist->children[index - 1];
		}

		dom::NodePtr nextSibling() override
		{
			dom::NodePtr par = parent.lock();
			if (!par)
				return dom::NodePtr();

			NodeImplInit* plist = (NodeImplInit*)par->internalData();
			if (!plist) return nullptr;

			size_t ndx = index + 1;
			if (ndx >= plist->children.size()) return dom::NodePtr();

			return plist->children[ndx];
		}

		dom::DocumentPtr ownerDocument() override { return document.lock(); }

		size_t indexOf(const NodePtr& node)
		{
			if (!node)
				return children.size();

			NodeImplInit* p = (NodeImplInit*)node->internalData();
			size_t index = p->index;
			if (index < children.size() && children[index] == node)
				return index;

			return children.size(); // same as if node was nullptr
		}

		bool insertBefore(const NodePtr& newChild, const NodePtr& before = nullptr) override
		{
			if (!newChild) return false;
			dom::DocumentPtr doc = newChild->ownerDocument();
			if (!doc || doc != document.lock()) return false;

			size_t index = indexOf(before); // in case newChild == before
			if (!removeFromParent(newChild))
				return false;

			if (newChild->nodeType() == ATTRIBUTE_NODE)
				return ((T*)this)->appendAttr(newChild);

			NodeImplInit* p = (NodeImplInit*)newChild->internalData();
			p->parent = ((T*)this)->shared_from_this();

			auto it = children.begin();
			std::advance(it, index);
			children.insert(it, newChild);

			p->fixQName();

			size_t length = children.size();
			for (size_t i = index; i < length; ++i)
			{
				auto child = children[i];
				NodeImplInit* p = (NodeImplInit*)child->internalData();
				p->index = i;
			}

			return true;
		}
		bool insertBefore(const NodeListPtr& children, const NodePtr& before = nullptr) override
		{
			if (!children)
				return false;

			std::vector<NodePtr> copy;
			copy.reserve(children->length());

			auto this_doc = document.lock();

			for (auto node : list_nodes(children))
			{
				if (!node)
					return false;
				dom::DocumentPtr doc = node->ownerDocument();
				if (!doc || doc != this_doc)
					return false;

				copy.push_back(node);
			}

			size_t index = indexOf(before); // in case any new child == before

			for (auto&& node : copy)
			{
				if (!removeFromParent(node))
					return false;
			}

			auto it = this->children.begin();
			std::advance(it, index);

			for (auto&& node : copy)
			{
				if (node->nodeType() == ATTRIBUTE_NODE)
				{
					if (!((T*)this)->appendAttr(node))
						return false;
					continue;
				}

				NodeImplInit* p = (NodeImplInit*)node->internalData();
				p->parent = ((T*)this)->shared_from_this();
				it = this->children.insert(it, node);
				++it;
				p->fixQName();
			}

			size_t length = this->children.size();
			for (size_t i = index; i < length; ++i)
			{
				auto child = this->children[i];
				NodeImplInit* p = (NodeImplInit*)child->internalData();
				p->index = i;
			}

			return true;
		}

		bool appendChild(const dom::NodePtr& newChild) override
		{
			return insertBefore(newChild);
		}
		bool replaceChild(const NodePtr& newChild, const NodePtr& oldChild) override
		{
			if (!oldChild)
				return false;

			if (!insertBefore(newChild, oldChild))
				return false;

			return removeChild(oldChild);
		}
		bool replaceChild(const NodeListPtr& newChildren, const NodePtr& oldChild) override
		{
			if (!oldChild)
				return false;

			if (!insertBefore(newChildren, oldChild))
				return false;

			return removeChild(oldChild);
		}

		bool removeChild(const NodePtr& child) override
		{
			if (!child)
				return false;

			if (child->nodeType() == dom::ATTRIBUTE_NODE)
				return ((T*)this)->removeAttr(child);

			index = indexOf(child);
			if (index >= children.size())
				return false;

			NodeImplInit* p = (NodeImplInit*)child->internalData();
			p->parent.reset();
			p->index = (size_t)-1;

			auto it = children.begin();
			std::advance(it, index);
			children.erase(it);
			size_t length = children.size();
			for (size_t i = index; i < length; ++i)
			{
				auto child = children[i];
				NodeImplInit* p = (NodeImplInit*)child->internalData();
				p->index = i;
			}

			return true;
		}

		void* internalData() override { return (NodeImplInit*)this; }

		bool appendAttr(const dom::NodePtr& newChild) { return false; }
		bool removeAttr(const dom::NodePtr& child) { return false; }

		NodePtr find(const std::string& path, const Namespaces& ns) override
		{
			return xpath::XPath(path, ns).find(((T*)this)->shared_from_this());
		}
		NodeListPtr findall(const std::string& path, const Namespaces& ns) override
		{
			return xpath::XPath(path, ns).findall(((T*)this)->shared_from_this());
		}
	};

}} // dom::impl

#endif // __DOM_INTERNAL_NODE_IMPL_HPP__