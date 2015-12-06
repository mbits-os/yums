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

#include <dom/parsers/parser.hpp>
namespace dom { namespace parsers {

#if 0
	DocumentPtr parseFile(const ParserPtr& parser, const filesystem::path& path)
	{
		if (!parser)
			return nullptr;

		FILE* f = fopen(path.native().c_str(), "rb");
		if (!f)
			return nullptr;

		if (!parser->supportsChunks())
		{
			size_t length = filesystem::file_size(path);
			char * contents = (char*)malloc(length);
			if (!contents)
				return nullptr;

			size_t read = fread(contents, 1, length, f);
			fclose(f);

			if (!parser->onData(contents, length))
			{
				free(contents);
				return nullptr;
			}

			free(contents);
			return parser->onFinish();
		}

		char buffer[8192];
		size_t read;
		while ((read = fread(buffer, 1, sizeof(buffer), f)) > 0)
		{
			if (!parser->onData(buffer, read))
			{
				fclose(f);
				return nullptr;
			}
		}
		fclose(f);

		return parser->onFinish();
	}
#endif
}}
