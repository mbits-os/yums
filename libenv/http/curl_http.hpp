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

#ifndef __CURL_HTTP_HPP__
#define __CURL_HTTP_HPP__

#include <http/http_logger.hpp>

namespace net { namespace http {
	struct HttpEndpoint;
	struct HttpCallback;
	using HttpEndpointPtr = std::shared_ptr<HttpEndpoint>;
	using HttpCallbackPtr = std::shared_ptr<HttpCallback>;

	typedef std::map< std::string, std::string > Headers;

	struct HttpEndpoint
	{
		virtual ~HttpEndpoint() {}
		virtual void send(bool async) = 0;
		virtual void releaseEndpoint() = 0;
		virtual void abort() = 0;
		virtual void appendHeader(const std::string& header) = 0;
	};

	struct HttpCallback
	{
		virtual ~HttpCallback() {}
		virtual void onStart() = 0;
		virtual void onError(const std::string& error) = 0;
		virtual void onFinish() = 0;
		virtual size_t onData(const void* data, size_t count) = 0;
		virtual void onFinalLocation(const std::string& location) = 0;
		virtual void onHeaders(const std::string& reason, int http_status, const Headers& headers) = 0;

		virtual void appendHeaders() = 0;
		virtual std::string getUrl() = 0;
		virtual std::string getUserAgent() = 0;
		virtual void* getContent(size_t& length) = 0;
		virtual std::shared_ptr<client::LoggingClient> getLogger() const = 0;
		virtual long getMaxRedirs() = 0;
		virtual bool shouldFollowLocation() = 0;
		virtual bool headersOnly() const = 0;
	};

	HttpEndpointPtr GetEndpoint(const HttpCallbackPtr&);
}}

namespace net { namespace ftp {
	struct FtpEndpoint;
	using FtpEndpointPtr = std::shared_ptr<FtpEndpoint>;

	struct FtpEndpoint
	{
		virtual ~FtpEndpoint() {}
		virtual void send(bool async) = 0;
		virtual void releaseEndpoint() = 0;
		virtual void abort() = 0;
	};

	FtpEndpointPtr GetEndpoint(const http::HttpCallbackPtr&);
}}

#endif //__CURL_HTTP_HPP__