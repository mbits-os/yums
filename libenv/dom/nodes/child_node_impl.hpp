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

#ifndef __DOM_INTERNAL_CHILD_NODE_IMPL_HPP__
#define __DOM_INTERNAL_CHILD_NODE_IMPL_HPP__

#include "node_impl.hpp"

namespace dom { namespace impl {

	template <typename T, typename _Interface>
	class ChildNodeImpl : public NodeImpl<T, _Interface>
	{
	public:

		typedef NodeImpl<T, _Interface> Super;
		ChildNodeImpl(const NodeImplInit& init) : Super(init)
		{
		}

		bool before(const NodePtr& node) override
		{
			auto parent = static_cast<T*>(this)->parentNode();
			if (!parent)
				return false;
			return parent->insertBefore(node, Super::shared_from_this());
		}
		bool before(const std::string& data) override
		{
			auto node = createTextSibling(this, data);
			if (!node) return false;
			return before(node);
		}
		bool before(const NodeListPtr& nodes) override
		{
			auto parent = static_cast<T*>(this)->parentNode();
			if (!parent)
				return false;
			return parent->insertBefore(nodes, Super::shared_from_this());
		}
		bool after(const NodePtr& node) override
		{
			auto parent = static_cast<T*>(this)->parentNode();
			if (!parent)
				return false;
			return parent->insertBefore(node, parent->nextSibling());
		}
		bool after(const std::string& data) override
		{
			auto node = createTextSibling(this, data);
			if (!node) return false;
			return after(node);
		}
		bool after(const NodeListPtr& nodes) override
		{
			auto parent = static_cast<T*>(this)->parentNode();
			if (!parent)
				return false;
			return parent->insertBefore(nodes, parent->nextSibling());
		}
		bool replace(const NodePtr& node) override
		{
			auto parent = static_cast<T*>(this)->parentNode();
			if (!parent)
				return false;
			return parent->replaceChild(node, Super::shared_from_this());
		}
		bool replace(const std::string& data) override
		{
			auto node = createTextSibling(this, data);
			if (!node) return false;
			return replace(node);
		}
		bool replace(const NodeListPtr& nodes) override
		{
			auto parent = static_cast<T*>(this)->parentNode();
			if (!parent)
				return false;
			return parent->replaceChild(nodes, Super::shared_from_this());
		}
		bool remove() override
		{
			auto parent = static_cast<T*>(this)->parentNode();
			if (!parent)
				return true; // orphaned nodeas are always removed
			return parent->removeChild(Super::shared_from_this());
		}
	};
}}

#endif // __DOM_INTERNAL_CHILD_NODE_IMPL_HPP__
