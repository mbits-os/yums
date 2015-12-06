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

#ifndef __DOM_ELEMENT_HPP__
#define __DOM_ELEMENT_HPP__

#include <dom/nodes/node.hpp>

namespace dom
{
	struct Element : ParentNode
	{
		virtual std::string tagName() const { return nodeName(); }
		virtual std::string stringValue() override { return innerText(); }
		virtual std::string getAttribute(const std::string& name) = 0;
		virtual AttributePtr getAttributeNode(const std::string& name) = 0;
		virtual bool setAttribute(const AttributePtr& attr) = 0;
		virtual bool removeAttribute(const AttributePtr& attr) = 0;
		virtual bool setAttribute(const std::string& attr, const std::string& value) = 0;
		virtual bool removeAttribute(const std::string& attr) = 0;
		virtual NodeListPtr getAttributes() = 0;
		virtual bool hasAttribute(const std::string& name) = 0;
		virtual NodeListPtr getElementsByTagName(const std::string& tagName) = 0;
		virtual std::string innerText() = 0;
	};
}

#endif // __DOM_ELEMENT_HPP__