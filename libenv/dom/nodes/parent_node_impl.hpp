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

#ifndef __DOM_INTERNAL_PARENT_NODE_IMPL_HPP__
#define __DOM_INTERNAL_PARENT_NODE_IMPL_HPP__

#include "child_node_impl.hpp"

namespace dom { namespace impl {

	template <typename T, typename _Interface>
	class ParentNodeImpl : public ChildNodeImpl<T, _Interface>
	{
	public:
		typedef ChildNodeImpl<T, _Interface> Super;
		ParentNodeImpl(const NodeImplInit& init) : Super(init)
		{
		}

		bool prepend(const NodePtr& node) override
		{
			auto _final = static_cast<T*>(this);
			return _final->insertBefore(node, _final->firstChild());
		}

		bool prepend(const NodeListPtr& nodes) override
		{
			auto _final = static_cast<T*>(this);
			return _final->insertBefore(nodes, _final->firstChild());
		}

		bool prepend(const std::string& data) override
		{
			auto node = createTextSibling(this, data);
			if (!node) return false;
			return prepend(node);
		}

		bool append(const NodePtr& node) override
		{
			return static_cast<T*>(this)->insertBefore(node);
		}

		bool append(const NodeListPtr& nodes) override
		{
			return static_cast<T*>(this)->insertBefore(nodes);
		}

		bool append(const std::string& data) override
		{
			auto node = createTextSibling(this, data);
			if (!node) return false;
			return append(node);
		}
	};
}}

#endif // __DOM_INTERNAL_PARENT_NODE_IMPL_HPP__