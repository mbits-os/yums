/*
 * Copyright (C) 2014 midnightBITS
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

#ifndef __HTTP_XHR_HPP__
#define __HTTP_XHR_HPP__

#include <string>
#include <memory>
#include <map>
#include <functional>
#include <future>
#include <http/http_logger.hpp>

namespace net { namespace http { namespace client {
	struct HttpResponse;
	struct XmlHttpRequest;
	using HttpResponsePtr = std::shared_ptr<HttpResponse>;
	using XmlHttpRequestPtr = std::shared_ptr<XmlHttpRequest>;

	using HTTPArgs = std::map<std::string, std::string>;

	enum HTTP_METHOD
	{
		HTTP_GET,
		HTTP_POST,
		HTTP_HEAD
	};

	//http://www.w3.org/TR/XMLHttpRequest/

	struct HttpResponse
	{
		virtual ~HttpResponse() {}
		virtual int getStatus() const = 0;
		virtual std::string getStatusText() const = 0;
		virtual std::string getResponseHeader(const std::string& name) const = 0;
		virtual std::map<std::string, std::string> getResponseHeaders() const = 0;

		virtual bool wasRedirected() const = 0;
		virtual const std::string getFinalLocation() const = 0;
	};

	struct XmlHttpRequest: HttpResponse
	{
		//static XmlHttpRequestPtr Create();

		enum READY_STATE
		{
			UNSENT = 0,
			OPENED = 1,
			HEADERS_RECEIVED = 2,
			LOADING = 3,
			DONE = 4
		};

		using ONREADYSTATECHANGE = std::function<void(XmlHttpRequest*)>;
		using ONPROGRESS = std::function<void(bool, uint64_t, uint64_t)>;

		virtual void onreadystatechange(ONREADYSTATECHANGE handler) = 0;
		virtual void onprogress(ONPROGRESS handler) = 0;
		virtual READY_STATE getReadyState() const = 0;

		virtual void open(HTTP_METHOD method, const std::string& url, bool async = true) = 0;
		virtual void setRequestHeader(const std::string& header, const std::string& value) = 0;
		virtual void setBody(const void* data, size_t length) = 0;

		virtual void send(const void* data, size_t length) { setBody(data, length); send(); } 
		virtual void send() = 0;
		virtual void abort() = 0;

		virtual size_t getResponseTextLength() const = 0;
		virtual const char* getResponseText() const = 0;

		virtual void setLogging(const std::shared_ptr<LoggingClient>& logger) = 0;
		virtual void setShouldFollowLocation(bool follow) = 0;
		virtual void setMaxRedirects(size_t redirects) = 0;

		virtual const std::string& getError() = 0;
	};

	XmlHttpRequestPtr create();

	void set_program_client_info(const char*);
}}}

#endif //__HTTP_HPP__