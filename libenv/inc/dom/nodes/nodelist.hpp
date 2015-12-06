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

#ifndef __DOM_NODELIST_HPP__
#define __DOM_NODELIST_HPP__

#include <dom/nodes/node.hpp>

namespace dom
{
	struct NodeList
	{
		virtual ~NodeList() {}
		virtual NodePtr item(size_t index) = 0;
		ElementPtr element(size_t index)
		{
			NodePtr i = item(index);
			if (i && i->nodeType() == ELEMENT_NODE)
				return std::static_pointer_cast<Element>(i);
			return ElementPtr();
		}
		TextPtr text(size_t index)
		{
			NodePtr i = item(index);
			if (i && i->nodeType() == TEXT_NODE)
				return std::static_pointer_cast<Text>(i);
			return TextPtr();
		}
		AttributePtr attr(size_t index)
		{
			NodePtr i = item(index);
			if (i && i->nodeType() == ATTRIBUTE_NODE)
				return std::static_pointer_cast<Attribute>(i);
			return nullptr;
		}
		virtual size_t length() const = 0;
		virtual bool remove() = 0;
	};
}

#endif // __DOM_NODELIST_HPP__