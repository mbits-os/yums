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

#include "curl_http.hpp"
#define CURL_STATICLIB
#include <curl/curl.h>
#include <cstring>
#include <sstream>

#include <string>
#include <cctype>
#include <thread>

namespace std
{
	ostream& operator<<(ostream& o, void(*fn)(ostream&))
	{
		fn(o);
		return o;
	}
	static std::string tolower(std::string in) {
		for (auto&& c : in)
			c = (char)std::tolower((unsigned char)c);
		return in;
	}
}

namespace net { namespace http {
	void Init()
	{
		curl_global_init(CURL_GLOBAL_ALL);
	}

	template <typename Final>
	struct CurlBase
	{
		typedef unsigned long long size_type;

		CURL* m_curl;

		CurlBase(): m_curl(nullptr)
		{
			m_curl = curl_easy_init();
		}
		virtual ~CurlBase()
		{
			if (m_curl)
				curl_easy_cleanup(m_curl);
			m_curl = nullptr;
		}
		explicit operator bool () const { return m_curl != nullptr; }

		std::string recentIP() const
		{
			char* ip = nullptr;
			curl_easy_getinfo(m_curl, CURLINFO_PRIMARY_IP, &ip);
			return ip;
		}

		long recentPort() const
		{
			long port = 0;
			curl_easy_getinfo(m_curl, CURLINFO_PRIMARY_PORT, &port);
			return port;
		}

		void followLocation()
		{
			curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(m_curl, CURLOPT_AUTOREFERER, 1L);
		}

		void dontFollowLocation()
		{
			curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 0L);
			curl_easy_setopt(m_curl, CURLOPT_AUTOREFERER, 0L);
			curl_easy_setopt(m_curl, CURLOPT_MAXREDIRS, -1L);
		}

		void setMaxRedirs(long redirs)
		{
			curl_easy_setopt(m_curl, CURLOPT_MAXREDIRS, redirs);
		}

		void setConnectTimeout(long timeout)
		{
			curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, timeout);
		}

		void setUrl(const std::string& url)
		{
			curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
		}

		void setUA(const std::string& userAgent)
		{
			curl_easy_setopt(m_curl, CURLOPT_USERAGENT, userAgent.c_str());
		}

		void setHeaders(curl_slist* headers)
		{
			curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headers);
		}

		void setPostData(const void* data, size_t length)
		{
			curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, length);
			curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, data);
		}

		void setSSLVerify(bool verify = true)
		{
			long val = verify ? 1 : 0;
			curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, val);
			curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, val);
			//curl_easy_setopt(m_curl, CURLOPT_CERTINFO, 1L);
			//curl_easy_setopt(m_curl, CURLOPT_SSL_CTX_FUNCTION, curl_ssl_ctx_callback); 
		}

		void setWrite()
		{
			auto _this = static_cast<Final*>(this);
			curl_easy_setopt(m_curl, CURLOPT_WRITEHEADER, _this);
			curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, curl_onHeader);
			curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, _this);
			curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, curl_onData);
		}

		void setProgress()
		{
			curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0L);
			curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, curl_onProgress);
			curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, static_cast<Final*>(this));
		}

		void setCredentials(const std::string& user, const std::string& password)
		{
			curl_easy_setopt(m_curl, CURLOPT_USERNAME, user.c_str());
			curl_easy_setopt(m_curl, CURLOPT_PASSWORD, password.c_str());
		}

		void setLogger(const std::shared_ptr<client::LoggingClient>& logger)
		{
			curl_easy_setopt(m_curl, CURLOPT_VERBOSE, logger ? 1L : 0L);
			if (logger)
			{
				curl_easy_setopt(m_curl, CURLOPT_DEBUGFUNCTION, curl_onTrace);
				curl_easy_setopt(m_curl, CURLOPT_DEBUGDATA, static_cast<Final*>(this));
			}
			else
			{
				curl_easy_setopt(m_curl, CURLOPT_DEBUGFUNCTION, nullptr);
				curl_easy_setopt(m_curl, CURLOPT_DEBUGDATA, nullptr);
			}
		}

		void setHeadersOnly()
		{
			curl_easy_setopt(m_curl, CURLOPT_NOBODY, 1);
		}

		CURLcode fetch()
		{
			return curl_easy_perform(m_curl);
		}

		std::string error(CURLcode err)
		{
			return curl_easy_strerror(err);
		}

	protected:
		size_type onData(const char* data, size_type length) { return 0; }
		size_type onHeader(const char* data, size_type length) { return 0; }
		size_type onUnderflow(void* data, size_type length) { return 0; }
		bool onProgress(double dltotal, double dlnow, double ultotal, double ulnow) { return true; }
		int onTrace(curl_infotype type, char *data, size_t size) { return 0; }

#define CURL_(name) static size_t curl_##name(const void * _Str, size_t _Size, size_t _Count, Final* _this) { return (size_t)(_this->name((const char*)_Str, (size_type)_Size * _Count) / _Size); }
		CURL_(onData);
		CURL_(onHeader);
#undef CURL_
		static size_t curl_onUnderflow(void * _Str, size_t _Size, size_t _Count, Final* _this) { return (size_t)(_this->onUnderflow(_Str, (size_type)_Size * _Count) / _Size); }
		static int curl_onProgress(Final* _this, double dltotal, double dlnow, double ultotal, double ulnow) { return _this->onProgress(dltotal, dlnow, ultotal, ulnow) ? 1 : 0; }
		static int curl_onTrace(CURL *, curl_infotype type, char *data, size_t size, Final *_this) { return _this->onTrace(type, data, size); }
	};

	class CurlHttpEndpoint;
	class HttpCurl: public CurlBase<HttpCurl>
	{
		int m_status;
		std::string m_statusText;
		std::string m_lastKey;
		Headers m_headers;
		bool m_headersLocked;
		std::weak_ptr<CurlHttpEndpoint> m_owner;
		std::weak_ptr<HttpCallback> m_callback;
		bool m_wasRedirected;
		std::string m_finalLocation;
		bool m_ignore401 = false;
		std::shared_ptr<client::LoggingClient> m_logger;

		void wasRedirected();

	public:
		HttpCurl()
			: m_status(0)
			, m_headersLocked(true)
			, m_wasRedirected(false)
		{
		}

		std::shared_ptr<CurlHttpEndpoint> getOwner() const { return m_owner.lock(); }
		void setOwner(const std::shared_ptr<CurlHttpEndpoint>& owner) { m_owner = owner; }
		std::shared_ptr<HttpCallback> getCallback() const { return m_callback.lock(); }
		void setCallback(const std::shared_ptr<HttpCallback>& callback) { m_callback = callback; }

		void setUrl(const std::string& url)
		{
			m_finalLocation = url;
			CurlBase<HttpCurl>::setUrl(url);
		}

		void setLogger(const std::shared_ptr<client::LoggingClient>& logger)
		{
			m_logger = logger;
			CurlBase<HttpCurl>::setLogger(logger);
		}

		inline bool isRedirect() const;
		void sendHeaders() const;
		void logHeaders() const;
		inline bool authenticationNeeded() const;
		std::string getRealm() const;
		void ignoreAuthProblems(bool ignore = true);

		size_type onData(const char* data, size_type length);
		size_type onHeader(const char* data, size_type length);
		size_type onUnderflow(void*, size_type) { return 0; } // still not implemented
		inline bool onProgress(double, double, double, double);
		int onTrace(curl_infotype type, char *data, size_t size);
	};

	class CurlHttpEndpoint : public http::HttpEndpoint, public std::enable_shared_from_this<CurlHttpEndpoint>
	{
		std::weak_ptr<HttpCallback> m_callback;
		bool aborting = false;
		curl_slist* headers = nullptr;
		HttpCurl m_curl;

	public:
		CurlHttpEndpoint(const HttpCallbackPtr& obj) : m_callback(obj) {}
		~CurlHttpEndpoint()
		{
			if (headers)
			{
				curl_slist_free_all(headers);
				headers = nullptr;
			}
		}
		void attachTo(const HttpCallbackPtr& obj) { m_callback = obj; }
		void send(bool async) override
		{
			aborting = false;
			if (headers)
			{
				curl_slist_free_all(headers);
				headers = nullptr;
			}

			auto cb = m_callback.lock();
			auto thiz = shared_from_this();
			if (async && cb)
				std::thread([thiz]{ thiz->run(); }).detach();
			else
				run();
		}
		void releaseEndpoint() override;
		void abort() override { aborting = true; }
		void appendHeader(const std::string& header) override
		{
			headers = curl_slist_append(headers, header.c_str());
		}

		bool isAborting() const { return aborting; }

		void run();
		std::weak_ptr<HttpCallback> callback() const { return m_callback; }
	};

	inline bool HttpCurl::onProgress(double, double, double, double)
	{
		auto owner = getOwner();
		if (!owner)
			return true;
		return owner->isAborting();
	}

	HttpEndpointPtr GetEndpoint(const HttpCallbackPtr& obj)
	{
		return std::make_shared<CurlHttpEndpoint>(obj);
	}

	void CurlHttpEndpoint::releaseEndpoint()
	{
		m_callback.reset();
		m_curl.setCallback(nullptr);
		//CurlModule::instance().makeAvailable(shared_from_this());
	}

	void CurlHttpEndpoint::run()
	{
		auto http_callback = m_callback.lock();
		if (!http_callback)
			return;

		http_callback->onStart();

		if (!m_curl)
		{
			http_callback->onError("libCurl handle not inited.");
			return;
		}

		m_curl.setCallback(http_callback);
		m_curl.setOwner(shared_from_this());
		m_curl.setConnectTimeout(30);
		m_curl.setUA(http_callback->getUserAgent());
		m_curl.setProgress();
		m_curl.setSSLVerify(false);
		m_curl.setWrite();

		http_callback->appendHeaders();

		m_curl.setUrl(http_callback->getUrl());
		m_curl.setHeaders(headers);

		auto logger = http_callback->getLogger();
		if (logger)
			logger->onStart(http_callback->getUrl());

		if (http_callback->shouldFollowLocation())
		{
			m_curl.followLocation();
			long redirs = http_callback->getMaxRedirs();
			if (redirs != 0)
				m_curl.setMaxRedirs(redirs);
		}
		else
		{
			m_curl.dontFollowLocation();
		}

		//auto cred = http_callback->getCredentials();
		//if (cred)
		//	m_curl.setCredentials(cred->username(), cred->password());

		m_curl.setLogger(logger);

		size_t length;
		void* content = http_callback->getContent(length);

		if (content && length)
		{
			//curl_httppost; HTTPPOST_CALLBACK;
			m_curl.setPostData(content, length);
		}

		if (http_callback->headersOnly())
			m_curl.setHeadersOnly();

		CURLcode ret = m_curl.fetch();

		if (ret == CURLE_COULDNT_RESOLVE_HOST) {
			using namespace std::chrono;
			std::this_thread::sleep_for(500ms);
			ret = m_curl.fetch();
		}

		//if (m_curl.authenticationNeeded()) {
		//	if (cred) {
		//		while (m_curl.authenticationNeeded()) {
		//			std::future<bool> revauth = cred->askUser(m_curl.recentIP() + ":" + std::to_string(m_curl.recentPort()), m_curl.getRealm());
		//			if (!revauth.get()) {
		//				m_curl.ignoreAuthProblems();
		//				ret = m_curl.fetch();
		//				break;
		//			}

		//			m_curl.setCredentials(cred->username(), cred->password());
		//			ret = m_curl.fetch();
		//		}
		//	}
		//}

		if (m_curl.isRedirect())
			m_curl.sendHeaders(); // we must have hit max or a circular

		if (ret == CURLE_OK)
			http_callback->onFinish();
		else
			http_callback->onError(http_callback->getUrl() + " error: " + m_curl.error(ret));

		if (logger)
			logger->onStop(ret == CURLE_OK);
	}

	namespace Transfer
	{
		template <bool equal>
		struct Data { static HttpCurl::size_type onData(const HttpCallbackPtr& http_callback, const char* data, HttpCurl::size_type length); };

		template <>
		struct Data<true> { 
			static HttpCurl::size_type onData(const HttpCallbackPtr& http_callback, const char* data, HttpCurl::size_type length)
			{
				return http_callback->onData(data, (size_t)length);
			}
		};

		template <>
		struct Data<false> { 
			static HttpCurl::size_type onData(const HttpCallbackPtr& http_callback, const char* data, HttpCurl::size_type length)
			{
				HttpCurl::size_type written = 0;
				while (length)
				{
					HttpCurl::size_type chunk = (size_t)-1;
					if (chunk > length) chunk = length;
					length -= chunk;
					size_t st_chunk = (size_t)chunk;
					size_t ret = http_callback->onData(data, st_chunk);
					data += st_chunk;
					written += ret;
					if (ret != st_chunk)
						break;
				}

				return written;
			}
		};

		static HttpCurl::size_type onData(const HttpCallbackPtr& http_callback, const char* data, HttpCurl::size_type length)
		{
			return Data<sizeof(HttpCurl::size_type) <= sizeof(size_t)>::onData(http_callback, data, length);
		}

		template <bool equal>
		struct String { static void append(std::string& out, const char* data, HttpCurl::size_type length); };

		template <>
		struct String<true> {
			static void append(std::string& out, const char* data, HttpCurl::size_type length)
			{
				out.append(data, (size_t)length);
			}
		};

		template <>
		struct String<false> {
			static void append(std::string& out, const char* data, HttpCurl::size_type length)
			{
				while (length)
				{
					HttpCurl::size_type chunk = (std::string::size_type)-1;
					if (chunk > length) chunk = length;
					length -= chunk;
					std::string::size_type st_chunk = (size_t)chunk;
					out.append(data, st_chunk);
					data += st_chunk;
				}
			}
		};

		static void appendString(std::string& out, const char* data, HttpCurl::size_type length)
		{
			return String<sizeof(HttpCurl::size_type) <= sizeof(std::string::size_type)>::append(out, data, length);
		}
	};

	inline bool HttpCurl::isRedirect() const
	{
		/*
		 * 01 
		 * 02 301 Moved Permanently
		 * 04 302 Found
		 * 08 303 See Other
		 * 10 
		 * 20 
		 * 40 
		 * 80 307 Temporary Redirect
		 */
		static int codes = 0x8E;
		static int max_redirect = 7;
#define BIT_POS (m_status % 100)
#define BIT (1 << BIT_POS)
		return m_status / 100 == 3 ? ( BIT_POS <= max_redirect ? (BIT & codes) == BIT : false) : false;
	}

	inline bool HttpCurl::authenticationNeeded() const
	{
		return !m_ignore401 && (m_status == 401);
	}

	struct WWWAuthenticate {
		std::string challenge;
		std::map<std::string, std::string> params;
	};

#define SP while (cur != end && std::isspace((uint8_t)*cur)) ++cur;
#define NSP while (cur != end && !std::isspace((uint8_t)*cur) && *cur != '=') ++cur;

	bool http_token(std::string::const_iterator& cur, const std::string::const_iterator& end, std::string& token)
	{
		auto save = cur;
		SP;
		auto start = cur;
		NSP;
		token.assign(start, cur);
		return save != cur;
	}

	bool http_quoted(std::string::const_iterator& cur, const std::string::const_iterator& end, std::string& str)
	{
		SP;
		if (cur == end || *cur != '"')
			return false;
		++cur;

		bool in_esc = false;
		while (cur != end) {
			if (in_esc) {
				in_esc = false;
				str.push_back(*cur);
			} else {
				if (*cur == '"') {
					++cur;
					return true;
				}
				if (*cur == '\\')
					in_esc = true;
				else
					str.push_back(*cur);
			}
			++cur;
		}

		return false;
	}

	bool http_param(std::string::const_iterator& cur, const std::string::const_iterator& end, std::pair<std::string, std::string>& param)
	{
		if (!http_token(cur, end, param.first))
			return false;

		SP;
		if (cur == end || *cur != '=')
			return false;
		++cur;

		SP;
		if (cur != end && *cur == '"') 
			return http_quoted(cur, end, param.second);
		else
			return http_token(cur, end, param.second);
	}

	WWWAuthenticate parse_auth(const std::string& header)
	{
		WWWAuthenticate out;
		auto cur = std::begin(header);
		auto end = std::end(header);
		if (!http_token(cur, end, out.challenge))
			return { };

		std::pair<std::string, std::string> param;
		while (http_param(cur, end, param)) {
			out.params[std::tolower(param.first)] = std::move(param.second);

			SP;
			if (cur == end || *cur != ',')
				break;
		}

		return out;
	}

	std::string HttpCurl::getRealm() const
	{
		auto it = m_headers.find("www-authenticate");
		if (it == m_headers.end())
			return { };

		auto auth = parse_auth(it->second);
		it = auth.params.find("realm");
		if (it == auth.params.end())
			return { };

		return it->second;
	}

	void HttpCurl::ignoreAuthProblems(bool ignore)
	{
		m_ignore401 = ignore;
	}

	HttpCurl::size_type HttpCurl::onData(const char* data, size_type length)
	{
		// Redirects should not have bodies anyway
		// And if we redirect, there will be a new header soon...
		// Same for login issues.
		if (isRedirect() || authenticationNeeded()) return length;

		auto callback = getCallback();
		if (!callback)
			return 0;
		return Transfer::onData(callback, data, length);
	}

#define C_WS while (read < length && isspace((unsigned char)*data)) ++data, ++read;
#define C_NWS while (read < length && !isspace((unsigned char)*data)) ++data, ++read;

	HttpCurl::size_type HttpCurl::onHeader(const char* data, size_type length)
	{
		size_t read = 0;

		bool rn_present = false;
		if (length > 1 && data[length-2] == '\r' && data[length-1] == '\n')
		{
			length -= 2;
			rn_present = true;
		}

		///////////////////////////////////////////////////////////////////////////////////
		//
		//                    FIRST LINE
		//
		///////////////////////////////////////////////////////////////////////////////////
		if (m_headersLocked)
		{
			if (m_status != 0)
			{
				wasRedirected();
			}

			m_headersLocked = false;
			m_status = 0;
			m_statusText.clear();
			m_lastKey.clear();
			m_headers.clear();

			C_NWS;
			C_WS;

			while (read < length && isdigit((unsigned char)*data))
			{
				m_status *= 10;
				m_status += *data - '0';
				++data, ++read;
			}
			if (!isspace((unsigned char)*data)) return read;

			C_WS;
			Transfer::appendString(m_statusText, data, length - read);

			if (rn_present)
				length += 2; // move back the \r\n

			return length;
		}

		///////////////////////////////////////////////////////////////////////////////////
		//
		//                    TERMINATOR
		//
		///////////////////////////////////////////////////////////////////////////////////
		if (length == 0 && rn_present)
		{
			m_headersLocked = true;
			if (!isRedirect() && !authenticationNeeded())
				sendHeaders();
			else
				logHeaders(); // in case it was a redir or 401, at least show'em in log
			return 2;
		}

		///////////////////////////////////////////////////////////////////////////////////
		//
		//                    CONTINUATION
		//
		///////////////////////////////////////////////////////////////////////////////////
		if (length && isspace((unsigned char)*data))
		{
			C_WS;
			std::string& lastValue = m_headers[m_lastKey];
			lastValue += ' ';
			Transfer::appendString(lastValue, data, length - read);

			if (rn_present)
				length += 2; // move back the \r\n

			return length;
		}

		///////////////////////////////////////////////////////////////////////////////////
		//
		//                    ANY OTHER LINE
		//
		///////////////////////////////////////////////////////////////////////////////////
		auto mark = read;
		auto ptr = data;

		while (mark < length && *ptr != ':') ++mark, ++ptr;
		if (mark == length)
		{
			if (rn_present)
				length += 2; // move back the \r\n

			return length;
		}

		auto pos = mark;
		auto last = ptr;

		//rewind
		while (pos > read && isspace((unsigned char)last[-1])) --last, --pos;
		if (pos == read)
		{
			if (rn_present)
				length += 2; // move back the \r\n

			return length;
		}
		std::string key;
		Transfer::appendString(key, data, pos - read);
		key = std::tolower(key);

		read = mark + 1;
		data = ptr + 1;
		C_WS;

		auto _it = m_headers.find(key);
		if (_it != m_headers.end()) {
			_it->second += ", ";
			Transfer::appendString(_it->second, data, length - read);
		} else
			Transfer::appendString(m_headers[key], data, length - read);

		if (rn_present)
			length += 2; // move back the \r\n

		return length;
	}

	void HttpCurl::wasRedirected()
	{
		m_wasRedirected = true;
		auto it = m_headers.find("location");
		if (it != m_headers.end())
		{
			//the spec says it should be a full URL, but there should be validation anyway here...
			m_finalLocation = it->second;
		}
	}

	void HttpCurl::sendHeaders() const
	{
		auto callback = getCallback();
		if (!callback)
			return;

		if (m_wasRedirected)
			callback->onFinalLocation(m_finalLocation);

		callback->onHeaders(m_statusText, m_status, m_headers);
		logHeaders();
	}

	void HttpCurl::logHeaders() const
	{
		if (!m_logger)
			return;
		m_logger->onResponse(m_statusText, m_status, m_headers);
		if (m_wasRedirected)
			m_logger->onFinalLocation(m_finalLocation);
	}

	int HttpCurl::onTrace(curl_infotype type, char *data, size_t size)
	{
		using http::client::trace;

		trace kind = trace::unknown;

		switch (type)
		{
		case CURLINFO_TEXT:
			if (m_logger)
				m_logger->onDebug(data);
			return 0;
		case CURLINFO_HEADER_OUT:
			if (m_logger)
				m_logger->onRequestHeaders(data, size);
			return 0;
		case CURLINFO_DATA_OUT:
			kind = trace::data_out;
			break;
		case CURLINFO_SSL_DATA_OUT:
			kind = trace::ssl_out;
			break;
		case CURLINFO_HEADER_IN:
			return 0; // will be handled in onHeaders
		case CURLINFO_DATA_IN:
			kind = trace::data_in;
			break;
		case CURLINFO_SSL_DATA_IN:
			kind = trace::data_in;
			break;
		default:
			break;
		}

		if (kind != trace::unknown && m_logger)
			m_logger->onTrace(kind, data, size);

		return 0;
	}

	namespace client {
		std::string http_client_info()
		{
#ifdef CURL_FULL_VERSION
			return curl_version();
#else
			auto version = curl_version_info(CURLVERSION_FIRST);
			return std::string{"libcurl/"} + version->version;
#endif
		}
	}
}}

namespace net { namespace ftp {
	class CurlFtpEndpoint;
	class FtpCurl: public http::CurlBase<FtpCurl>
	{
		std::weak_ptr<CurlFtpEndpoint> m_owner;
		std::weak_ptr<http::HttpCallback> m_callback;
		std::shared_ptr<http::client::LoggingClient> m_logger;

	public:
		FtpCurl()
		{
		}

		std::shared_ptr<CurlFtpEndpoint> getOwner() const { return m_owner.lock(); }
		void setOwner(const std::shared_ptr<CurlFtpEndpoint>& owner) { m_owner = owner; }
		std::shared_ptr<http::HttpCallback> getCallback() const { return m_callback.lock(); }
		void setCallback(const std::shared_ptr<http::HttpCallback>& callback) { m_callback = callback; }

		void setWrite()
		{
			curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
			curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, curl_onData);
		}

		void setLogger(const std::shared_ptr<http::client::LoggingClient>& logger)
		{
			m_logger = logger;
			CurlBase<FtpCurl>::setLogger(logger);
		}

		size_type onData(const char* data, size_type length);
		size_type onUnderflow(void*, size_type) { return 0; } // still not implemented
		inline bool onProgress(double, double, double, double);
		int onTrace(curl_infotype type, char *data, size_t size);
	};

	class CurlFtpEndpoint : public ftp::FtpEndpoint, public std::enable_shared_from_this<CurlFtpEndpoint>
	{
		std::weak_ptr<http::HttpCallback> m_callback;
		bool aborting = false;
		FtpCurl m_curl;

	public:
		CurlFtpEndpoint(const http::HttpCallbackPtr& obj) : m_callback(obj) {}
		void attachTo(const http::HttpCallbackPtr& obj) { m_callback = obj; }
		void send(bool async) override
		{
			aborting = false;

			auto cb = m_callback.lock();
			auto thiz = shared_from_this();
			if (async && cb)
				std::thread([thiz]{ thiz->run(); }).detach();
			else
				run();
		}
		void releaseEndpoint() override;
		void abort() override { aborting = true; }
		bool isAborting() const { return aborting; }

		void run();
		std::weak_ptr<http::HttpCallback> callback() const { return m_callback; }
	};


	inline bool FtpCurl::onProgress(double, double, double, double)
	{
		auto owner = getOwner();
		if (!owner)
			return true;
		return owner->isAborting();
	}

	FtpCurl::size_type FtpCurl::onData(const char* data, size_type length)
	{
		auto callback = getCallback();
		if (!callback)
			return 0;
		return http::Transfer::onData(callback, data, length);
	}

	int FtpCurl::onTrace(curl_infotype type, char *data, size_t size)
	{
		using http::client::trace;

		trace kind = trace::unknown;

		switch (type) {
		case CURLINFO_TEXT:
			if (m_logger)
				m_logger->onDebug(data);
			return 0;
		case CURLINFO_HEADER_OUT:
			if (m_logger)
				m_logger->onRequestHeaders(data, size);
			return 0;
		case CURLINFO_DATA_OUT:
			kind = trace::data_out;
			break;
		case CURLINFO_SSL_DATA_OUT:
			kind = trace::ssl_out;
			break;
		case CURLINFO_HEADER_IN:
			return 0; // will be handled in onHeaders
		case CURLINFO_DATA_IN:
			kind = trace::data_in;
			break;
		case CURLINFO_SSL_DATA_IN:
			kind = trace::data_in;
			break;
		default:
			break;
		}

		if (kind != trace::unknown && m_logger)
			m_logger->onTrace(kind, data, size);

		return 0;
	}

	FtpEndpointPtr GetEndpoint(const http::HttpCallbackPtr& obj)
	{
		return std::make_shared<CurlFtpEndpoint>(obj);
	}

	void CurlFtpEndpoint::releaseEndpoint()
	{
		m_callback.reset();
		m_curl.setCallback(nullptr);
		//CurlModule::instance().makeAvailable(shared_from_this());
	}

	void CurlFtpEndpoint::run()
	{
		auto ftp_callback = m_callback.lock();
		if (!ftp_callback)
			return;

		ftp_callback->onStart();

		if (!m_curl) {
			ftp_callback->onError("libCurl handle not inited.");
			return;
		}

		m_curl.setCallback(ftp_callback);
		m_curl.setOwner(shared_from_this());
		m_curl.setConnectTimeout(30);
		m_curl.setProgress();
		m_curl.setWrite();

		m_curl.setUrl(ftp_callback->getUrl());

		auto logger = ftp_callback->getLogger();
		if (logger)
			logger->onStart(ftp_callback->getUrl());

		m_curl.setLogger(logger);

		size_t length;
		void* content = ftp_callback->getContent(length);

		if (content && length) {
			//curl_httppost; HTTPPOST_CALLBACK;
			m_curl.setPostData(content, length);
		}

		CURLcode ret = m_curl.fetch();

		if (ret == CURLE_OK)
			ftp_callback->onFinish();
		else
			ftp_callback->onError(ftp_callback->getUrl() + " error: " + m_curl.error(ret));

		if (logger)
			logger->onStop(ret == CURLE_OK);
	}
}}
