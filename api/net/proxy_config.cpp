/**
 * Appcelerator Kroll - licensed under the Apache Public License 2  
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../kroll.h"
#include "net.h"

using std::string;
using std::vector;
using Poco::Net::IPAddress;
using Poco::URI;

namespace kroll
{
	static inline bool EndsWith(string haystack, string needle)
	{
		return haystack.find(needle) == (haystack.size() - needle.size());
	}

	bool Proxy::IsIPAddress(string& str)
	{
		IPAddress address;
		return IPAddress::tryParse(str, address);
	}

	bool Proxy::ShouldBypass(URI& uri)
	{
		const std::string& uriHost = uri.getHost();
		const std::string& uriScheme = uri.getScheme();
		unsigned short uriPort = uri.getPort();
		for (size_t i = 0; i < bypassList.size(); i++)
		{
			SharedURI entry = bypassList.at(i);

			// An empty bypass entry equals an unconditional bypass.
			if (entry.isNull())
			{
				return true;
			}
			else
			{
				const std::string& entryHost = entry->getHost();
				const std::string& entryScheme = entry->getScheme();
				unsigned short entryPort = entry->getPort();

				if (entryHost == "<local>" && uriHost.find(".") == string::npos)
				{
					return true;
				}
				else if (EndsWith(uriHost, entryHost) &&
					(entryScheme.empty() || entryScheme == uriScheme) &&
					(entryPort == 0 || entryPort == uriPort))
				{
					return true;
				}
			}
		}

		return false;
	}

	SharedURI Proxy::ProxyEntryToURI(string& entry)
	{
		SharedURI uri = new URI();

		size_t endScheme = entry.find("://");
		if (endScheme != string::npos)
		{
			uri->setScheme(entry.substr(0, endScheme));
			entry = entry.substr(endScheme + 3);
		}

		size_t scan = entry.size() - 1;
		while (scan > 0 && isdigit(entry[scan]))
			scan--;

		if (entry[scan] == ':' && scan != entry.size() - 1)
		{
			string portString = entry.substr(scan + 1);
			uri->setPort(atoi(portString.c_str()));
			entry = entry.substr(0, scan);
		}
		else
		{
			// Poco may have adjusted the port when we set the scheme
			// so force an accepting port here.
			uri->setPort(0);
		}

		uri->setHost(entry);
		return uri;
	}

	namespace ProxyConfig
	{
		SharedProxy httpProxyOverride(0);
		SharedProxy httpsProxyOverride(0);

		void SetHTTPProxyOverride(SharedProxy newProxyOverride)
		{
			httpProxyOverride = newProxyOverride;
		}

		SharedProxy GetHTTPProxyOverride()
		{
			return httpProxyOverride;
		}

		void SetHTTPSProxyOverride(SharedProxy newProxyOverride)
		{
			httpsProxyOverride = newProxyOverride;
		}

		SharedProxy GetHTTPSProxyOverride()
		{
			return httpsProxyOverride;
		}

		SharedProxy GetProxyForURL(string& url)
		{
			URI uri(url);

			// Don't try to detect proxy settings for URLs we know are local
			std::string scheme(uri.getScheme());
			if (scheme == "app" || scheme == "ti" || scheme == "file")
				return 0;

			if (scheme == "http" && !httpProxyOverride.isNull())
				return httpProxyOverride;

			if (scheme == "https" && !httpsProxyOverride.isNull())
				return httpsProxyOverride;

			return ProxyConfig::GetProxyForURLImpl(uri);
		}
	}
}
