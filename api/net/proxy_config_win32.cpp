/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../kroll.h"
#include "net.h"
#define _WINSOCKAPI_
#include <winsock2.h>
#include <windows.h>
#include <new.h>
#include <objbase.h>
#include <winhttp.h>
using std::string;
using std::wstring;
using std::vector;
using Poco::URI;
using Poco::StringTokenizer;

namespace kroll
{

	static void InitializeWin32ProxyConfig();
	static bool GetAutoProxiesForURL(string& url, vector<SharedProxy>& proxies);
	static void ParseProxyList(string proxyList, string bypassList,
		vector<SharedPtr<Proxy > >& ieProxyList);
	static std::string ErrorCodeToString(DWORD code);

	class WinHTTPSession
	{
		public:
		WinHTTPSession() :
			handle(NULL)
		{
			// Just use something reasonable as a user agent.
			this->handle = WinHttpOpen(
				L"Mozilla/5.0 (Windows; U; Windows NT 5.1; en) "
				L"AppleWebKit/526.9 (KHTML, like Gecko) "
				L"Version/4.0dp1 Safari/526.8",
				WINHTTP_ACCESS_TYPE_NO_PROXY,
				WINHTTP_NO_PROXY_NAME, 
				WINHTTP_NO_PROXY_BYPASS, 0);
		}

		inline HINTERNET GetHandle()
		{
			return handle;
		}

		~WinHTTPSession()
		{
			if (handle)
				WinHttpCloseHandle(handle);
		}

		private:
		HINTERNET handle;
	};

	bool useProxyAutoConfig = false;
	std::wstring autoConfigURL(L"");
	WinHTTPSession httpSession;
	std::vector<SharedProxy> ieProxies;

	static void InitializeWin32ProxyConfig()
	{
		static bool initialized = false;
		if (initialized)
			return;

		WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig;
		ZeroMemory(&ieProxyConfig, sizeof(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG)); 
		
		if (WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig))
		{
			if (ieProxyConfig.fAutoDetect)
			{
				useProxyAutoConfig = true;
			}
	
			if (ieProxyConfig.lpszAutoConfigUrl != NULL)
			{
				// We using an auto proxy configuration, but this one
				// has a URL which we must contact to get the configuration info.
				autoConfigURL = ieProxyConfig.lpszAutoConfigUrl;
			}

			// We always keep IE proxy information in case auto proxy
			// determination fails. We want to it as a fallback.
			if (ieProxyConfig.lpszProxy)
			{
				std::string bypassList;
				if (ieProxyConfig.lpszProxyBypass)
				{
					std::wstring bypassW = ieProxyConfig.lpszProxyBypass;
					bypassList = string(bypassW.begin(), bypassW.end());
				}

				std::wstring proxyListW = ieProxyConfig.lpszProxy;
				string proxyList = string(proxyListW.begin(), proxyListW.end());
				ParseProxyList(proxyList, bypassList, ieProxies);
			}
		}
		else
		{
			// If there is no IE configuration information, we default to
			// attempting to get auto proxy information.
			useProxyAutoConfig = true;
		}

		if (useProxyAutoConfig || !autoConfigURL.empty())
		{
			// We failed to open an HINTERNET handle! WTF. We'll have to have
			// disable auto proxy support, because we can't do a lookup.
			if (!httpSession.GetHandle())
			{
				useProxyAutoConfig = false;
				autoConfigURL = L"";
			}
		}

		if (ieProxyConfig.lpszProxy)
			GlobalFree(ieProxyConfig.lpszProxy);
		if (ieProxyConfig.lpszProxyBypass)
			GlobalFree(ieProxyConfig.lpszProxyBypass);
		if (ieProxyConfig.lpszAutoConfigUrl)
			GlobalFree(ieProxyConfig.lpszAutoConfigUrl);
	}

	// This method will return true if we should keep attempting to use a proxy
	// or false if the auto proxy determination was to use a direct connection.
	static bool GetAutoProxiesForURL(string& url, vector<SharedProxy>& proxies)
	{
		bool shouldUseProxy = true;
		WINHTTP_PROXY_INFO autoProxyInfo;
		ZeroMemory(&autoProxyInfo, sizeof(WINHTTP_PROXY_INFO)); 
		
		WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions;
		ZeroMemory(&autoProxyOptions, sizeof(WINHTTP_AUTOPROXY_OPTIONS)); 
		
		// This type of auto-detection might take several seconds, so
		// if the user specified  an autoconfiguration URL don't do it.
		// TODO: Maybe we should use this as a fallback later, but it's *very* expensive.
		if (autoConfigURL.empty() && useProxyAutoConfig)
		{
			autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
			autoProxyOptions.dwAutoDetectFlags = 
				WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
		}

		if (!autoConfigURL.empty())
		{
			autoProxyOptions.dwFlags |= WINHTTP_AUTOPROXY_CONFIG_URL;
			autoProxyOptions.lpszAutoConfigUrl = autoConfigURL.c_str();
		}
		
		// From Chromium:
		// Per http://msdn.microsoft.com/en-us/library/aa383153(VS.85).aspx, it is
		// necessary to first try resolving with fAutoLogonIfChallenged set to false.
		// Otherwise, we fail over to trying it with a value of true.  This way we
		// get good performance in the case where WinHTTP uses an out-of-process
		// resolver.  This is important for Vista and Win2k3.
		wstring wideURL = UTF8ToWide(url);
		autoProxyOptions.fAutoLogonIfChallenged = FALSE;
		BOOL ok = WinHttpGetProxyForUrl(
			httpSession.GetHandle(), wideURL.c_str(), &autoProxyOptions, &autoProxyInfo);

		if (!ok && ERROR_WINHTTP_LOGIN_FAILURE == GetLastError())
		{
			autoProxyOptions.fAutoLogonIfChallenged = TRUE;
			ok = WinHttpGetProxyForUrl(
				httpSession.GetHandle(), wideURL.c_str(), &autoProxyOptions, &autoProxyInfo);
		}
		
		if (ok && autoProxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NAMED_PROXY &&
			 autoProxyInfo.lpszProxy)
		{
			// Only the first proxy in the list will get a copy of the bypass list.
			std::string bypassList;
			if (autoProxyInfo.lpszProxyBypass)
			{
				std::wstring bypassW = autoProxyInfo.lpszProxyBypass;
				bypassList = string(bypassW.begin(), bypassW.end());
			}

			std::wstring proxyListW = autoProxyInfo.lpszProxy;
			string proxyList = string(proxyListW.begin(), proxyListW.end());
			ParseProxyList(proxyList, bypassList, proxies);
		}
		else if (ok && autoProxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NO_PROXY)
		{
			// The only case in which we do not continue to try using a proxy.
			// In this case the auto proxy setup told us to use a direct connection.
			shouldUseProxy = false;
		}
		else
		{
			// Auto proxy failed, so try another method
			string error = "Could not get proxy for url=";
			error.append(url);
			error.append(": ");
			error.append(ErrorCodeToString(GetLastError()));
			Logger::Get("Proxy")->Error(error);
		}
		
		// Always cleanup
		if (autoProxyInfo.lpszProxy)
			GlobalFree(autoProxyInfo.lpszProxy);
		if (autoProxyInfo.lpszProxyBypass)
			GlobalFree(autoProxyInfo.lpszProxyBypass);
		
		return shouldUseProxy;
	}

	static void ParseBypassList(string& bypassList,
		vector<SharedURI>& bypassVector)
	{
		std::string sep = ",";
		StringTokenizer tokenizer(bypassList, sep,
			StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
		for (size_t i = 0; i < tokenizer.count(); i++)
		{
			// Traditionally an endswith comparison is always done with the host
			// part, so we throwaway explicit wildcards at the beginning. If the
			// entire string is a wildcard this is an unconditional bypass.
			std::string entry = tokenizer[i];
			if (entry.at(0) == '*' && entry.size() == 1)
			{
				// Null URI means always bypass
				bypassVector.push_back(0);
				continue;
			}
			else if (entry.at(0) == '*' && entry.size() > 1)
			{
				entry = entry.substr(1);
			}

			SharedPtr<URI> bypassEntry = Proxy::ProxyEntryToURI(entry);
			bypassVector.push_back(bypassEntry);
		}
	}

	static void ParseProxyList(string proxyList, string bypassList,
		vector<SharedPtr<Proxy > >& ieProxyList)
	{
		std::string sep = "; ";
		StringTokenizer proxyTokens(proxyList, sep,
			StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
		for (size_t i = 0; i < proxyTokens.count(); i++)
		{
			string entry = proxyTokens[i];
			string scheme = "";
			bool forScheme = false;

			size_t schemeEnd = entry.find('=');
			if (schemeEnd != string::npos)
			{
				// This proxy only applies to the scheme before '='
				scheme = entry.substr(0, schemeEnd - 1);
				entry = entry.substr(schemeEnd + 1);
			}

			SharedPtr<Proxy> proxy = new Proxy;	
			proxy->info = Proxy::ProxyEntryToURI(entry);

			// Set the scheme based on the value that came before '='
			// We want this proxy to only apply to what it was specifically
			// set for.
			proxy->info->setScheme(scheme);
			ieProxyList.push_back(proxy);
		}

		if (ieProxyList.size() > 0 && !bypassList.empty())
		{
			SharedPtr<Proxy> proxy = ieProxyList.at(0);
			ParseBypassList(bypassList, proxy->bypassList);
		}
	}


	static std::string ErrorCodeToString(DWORD code)
	{
		// Okay. This is a little bit compulsive, but we really, really
		// want to get good debugging information for proxy lookup failures.
		if (code == ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR)
			return "ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR";
		else if (code == ERROR_WINHTTP_AUTODETECTION_FAILED)
			return "ERROR_WINHTTP_AUTODETECTION_FAILED";
		else if (code == ERROR_WINHTTP_BAD_AUTO_PROXY_SCRIPT)
			return "ERROR_WINHTTP_BAD_AUTO_PROXY_SCRIPT";
		else if (code == ERROR_WINHTTP_CANNOT_CALL_AFTER_OPEN)
			return "ERROR_WINHTTP_CANNOT_CALL_AFTER_OPEN";
		else if (code == ERROR_WINHTTP_CANNOT_CALL_AFTER_SEND)
			return "ERROR_WINHTTP_CANNOT_CALL_AFTER_SEND";
		else if (code == ERROR_WINHTTP_CANNOT_CALL_BEFORE_OPEN)
			return "ERROR_WINHTTP_CANNOT_CALL_BEFORE_OPEN";
		else if (code == ERROR_WINHTTP_CANNOT_CALL_BEFORE_SEND)
			return "ERROR_WINHTTP_CANNOT_CALL_BEFORE_OPEN";
		else if (code == ERROR_WINHTTP_CANNOT_CONNECT)
			return "ERROR_WINHTTP_CANNOT_CONNECT";
		else if (code == ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED)
			return "ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED";
		else if (code == ERROR_WINHTTP_CHUNKED_ENCODING_HEADER_SIZE_OVERFLOW)
			return "ERROR_WINHTTP_CHUNKED_ENCODING_HEADER_SIZE_OVERFLOW";
		else if (code == ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED)
			return "ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED";
		else if (code == ERROR_WINHTTP_CONNECTION_ERROR)
			return "ERROR_WINHTTP_CONNECTION_ERROR";
		else if (code == ERROR_WINHTTP_HEADER_ALREADY_EXISTS)
			return "ERROR_WINHTTP_HEADER_ALREADY_EXISTS";
		else if (code == ERROR_WINHTTP_HEADER_COUNT_EXCEEDED)
			return "ERROR_WINHTTP_HEADER_COUNT_EXCEEDED";
		else if (code == ERROR_WINHTTP_HEADER_NOT_FOUND)
			return "ERROR_WINHTTP_HEADER_NOT_FOUND";
		else if (code == ERROR_WINHTTP_HEADER_SIZE_OVERFLOW)
			return "ERROR_WINHTTP_HEADER_SIZE_OVERFLOW";
		else if (code == ERROR_WINHTTP_INCORRECT_HANDLE_STATE)
			return "ERROR_WINHTTP_INCORRECT_HANDLE_STATE";
		else if (code == ERROR_WINHTTP_INCORRECT_HANDLE_TYPE)
			return "ERROR_WINHTTP_INCORRECT_HANDLE_TYPE";
		else if (code == ERROR_WINHTTP_INTERNAL_ERROR)
			return "ERROR_WINHTTP_INTERNAL_ERROR";
		else if (code == ERROR_WINHTTP_INVALID_OPTION)
			return "ERROR_WINHTTP_INVALID_OPTION";
		else if (code == ERROR_WINHTTP_INVALID_QUERY_REQUEST)
			return "ERROR_WINHTTP_INVALID_QUERY_REQUEST";
		else if (code == ERROR_WINHTTP_INVALID_SERVER_RESPONSE)
			return "ERROR_WINHTTP_INVALID_SERVER_RESPONSE";
		else if (ERROR_WINHTTP_INVALID_URL)
			return "ERROR_WINHTTP_INVALID_URL";
		else if (ERROR_WINHTTP_LOGIN_FAILURE)
			return "ERROR_WINHTTP_LOGIN_FAILURE";
		else if (ERROR_WINHTTP_NAME_NOT_RESOLVED)
			return "ERROR_WINHTTP_NAME_NOT_RESOLVED";
		else if (ERROR_WINHTTP_NOT_INITIALIZED)
			return "ERROR_WINHTTP_NOT_INITIALIZED";
		else if (ERROR_WINHTTP_OPERATION_CANCELLED)
			return "ERROR_WINHTTP_OPERATION_CANCELLED";
		else if (ERROR_WINHTTP_OPTION_NOT_SETTABLE)
			return "ERROR_WINHTTP_OPTION_NOT_SETTABLE";
		else if (ERROR_WINHTTP_OUT_OF_HANDLES)
			return "ERROR_WINHTTP_OUT_OF_HANDLES";
		else if (ERROR_WINHTTP_REDIRECT_FAILED)
			return "ERROR_WINHTTP_REDIRECT_FAILED";
		else if (ERROR_WINHTTP_RESEND_REQUEST)
			return "ERROR_WINHTTP_RESEND_REQUEST";
		else if (ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW)
			return "ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW";
		else if (ERROR_WINHTTP_SECURE_CERT_CN_INVALID)
			return "ERROR_WINHTTP_SECURE_CERT_CN_INVALID";
		else if (ERROR_WINHTTP_SECURE_CERT_DATE_INVALID)
			return "ERROR_WINHTTP_SECURE_CERT_DATE_INVALID";
		else if (ERROR_WINHTTP_SECURE_CERT_REV_FAILED)
			return "ERROR_WINHTTP_SECURE_CERT_REV_FAILED";
		else if (ERROR_WINHTTP_SECURE_CERT_REVOKED)
			return "ERROR_WINHTTP_SECURE_CERT_REVOKED";
		else if (ERROR_WINHTTP_SECURE_CERT_WRONG_USAGE)
			return "ERROR_WINHTTP_SECURE_CERT_WRONG_USAGE";
		else if (ERROR_WINHTTP_SECURE_CHANNEL_ERROR)
			return "ERROR_WINHTTP_SECURE_CHANNEL_ERROR";
		else if (ERROR_WINHTTP_SECURE_FAILURE)
			return "ERROR_WINHTTP_SECURE_FAILURE";
		else if (ERROR_WINHTTP_SECURE_INVALID_CA)
			return "ERROR_WINHTTP_SECURE_INVALID_CA";
		else if (ERROR_WINHTTP_SECURE_INVALID_CERT)
			return "ERROR_WINHTTP_SECURE_INVALID_CERT";
		else if (ERROR_WINHTTP_SHUTDOWN)
			return "ERROR_WINHTTP_SHUTDOWN";
		else if (ERROR_WINHTTP_TIMEOUT)
			return "ERROR_WINHTTP_TIMEOUT";
		else if (ERROR_WINHTTP_UNABLE_TO_DOWNLOAD_SCRIPT)
			return "ERROR_WINHTTP_UNABLE_TO_DOWNLOAD_SCRIPT";
		else if (ERROR_WINHTTP_UNRECOGNIZED_SCHEME)
			return "ERROR_WINHTTP_UNRECOGNIZED_SCHEME";
		else if (ERROR_NOT_ENOUGH_MEMORY)
			return "ERROR_NOT_ENOUGH_MEMORY";
		else if (ERROR_INSUFFICIENT_BUFFER)
			return "ERROR_INSUFFICIENT_BUFFER";
		else if (ERROR_INVALID_HANDLE)
			return "ERROR_INVALID_HANDLE";
		else if (ERROR_NO_MORE_FILES)
			return "ERROR_NO_MORE_FILES";
		else if (ERROR_NO_MORE_ITEMS)
			return "ERROR_NO_MORE_ITEMS";
		else
			return "UNKNOWN ERROR";
	}

	namespace ProxyConfig
	{
		SharedPtr<Proxy> GetProxyForURLImpl(URI& uri)
		{
			InitializeWin32ProxyConfig();
			std::string url(uri.toString());

			// The auto proxy configuration might tell us to simply use
			// a direct connection, which should cause us to just return
			// null. Otherwise we should try to use the IE proxy list (next block)
			if (useProxyAutoConfig || !autoConfigURL.empty())
			{
				std::vector<SharedProxy> autoProxies;
				bool shouldUseIEProxy = GetAutoProxiesForURL(url, autoProxies);

				for (int i = 0; i < autoProxies.size(); i++)
				{
					SharedProxy proxy = autoProxies.at(i);
					if (proxy->ShouldBypass(uri))
					{
						return 0;
					}
					else if (proxy->info->getScheme().empty() ||
						proxy->info->getScheme() == uri.getScheme())
					{
						return proxy;
					}
				}

				if (!shouldUseIEProxy)
					return 0;
			}

			// Try the IE proxy list
			for (int i = 0; i < ieProxies.size(); i++)
			{
				SharedProxy proxy = ieProxies.at(i);
				std::string proxyScheme = proxy->info->getScheme();
				if (proxy->ShouldBypass(uri))
				{
					return 0;
				}
				else if (proxyScheme.empty() || proxyScheme == uri.getScheme())
				{
					return proxy;
				}
			}

			return 0;
		}
	}
}
