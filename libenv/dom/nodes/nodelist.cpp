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

#include "nodelist.hpp"
#include "node_impl.hpp"

namespace dom { namespace impl {

	using ListOfNodes = std::vector< dom::NodePtr >;

	NodeList::NodeList(const ListOfNodes& init) : children(init) {}

	dom::NodePtr NodeList::item(size_t index)
	{
		if (index >= children.size()) return dom::NodePtr();
		return children[index];
	}

	size_t NodeList::length() const { return children.size(); }

	bool NodeList::remove()
	{
		auto copy = children;
		for (auto&& node : copy)
		{
			if (!removeFromParent(node))
				return false;
		}

		return true;
	}

}}

namespace dom {
	NodeListPtr createList(const impl::NodePtrs& list)
	{
		try {
			return std::make_shared<impl::NodeList>(list);
		}
		catch (std::bad_alloc) { return nullptr; }
	}
}
