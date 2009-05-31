/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "utils.h"
#define OVERRIDE_ARG "--bundled-component-override"

using std::string;
using std::vector;
using std::pair;

namespace UTILS_NS
{
	extern void ScanRuntimesAtPath(string, vector<SharedComponent>&);
	extern void ScanModulesAtPath(string, vector<SharedComponent>&);
	extern void ScanBundledComponents(string, vector<SharedComponent>&);

	SharedPtr<Application> Application::NewApplication(string appPath)
	{
		string manifest = FileUtils::Join(appPath.c_str(), MANIFEST_FILENAME, NULL);
		return Application::NewApplication(manifest, appPath);
	}

	SharedPtr<Application> Application::NewApplication(
		string manifestPath,
		string appPath)
	{
		vector<pair<string, string> > manifest =
			BootUtils::ReadManifestFile(manifestPath);
		if (manifest.empty())
		{
			return NULL;
		}

		Application* application = new Application(appPath, manifestPath);
		vector<pair<string, string> >::iterator i = manifest.begin();
		while (i != manifest.end())
		{
			string key = i->first;
			string value = i->second;
			*i++;

			if (key == "#appname")
			{
				application->name = value;
				continue;
			}
			else if (key == "#appid")
			{
				application->id = value;
				continue;
			}
			else if (key == "#guid")
			{
				application->guid = value;
				continue;
			}
			else if (key == "#image")
			{
				application->image =
					 FileUtils::Join(application->GetResourcesPath().c_str(), value.c_str(), NULL);
				continue;
			}
			else if (key == "#publisher")
			{
				application->publisher = value;
				continue;
			}
			else if (key == "#url")
			{
				application->url = value;
				continue;
			}
			else if (key == "#version")
			{
				application->version = value;
				continue;
			}
			else if (key == "#stream")
			{
				application->stream = value;
				continue;
			}
			else if (key[0] == '#')
			{
				continue;
			}
			else
			{
				SharedDependency d = Dependency::NewDependency(key, value);
				application->dependencies.push_back(d);
			}
		}
		return application;
	}

	Application::Application(string path, string manifestPath) :
		path(path),
		manifestPath(manifestPath),
		stream("production")
	{
		// Default stream is production and is optional and doesn't have to be 
		// in the manifest unless switching from production to test or dev
	}

	Application::~Application()
	{
		this->modules.clear();
		this->runtime = NULL;
	}

	string Application::GetExecutablePath()
	{
		string exeName = this->name + ".exe";
		string path = FileUtils::Join(this->path.c_str(), exeName.c_str(), NULL);
		if (FileUtils::IsFile(path))
		{
			return path;
		}

		path = FileUtils::Join(this->path.c_str(), "MacOS", this->name.c_str(), NULL);
		if (FileUtils::IsFile(path))
		{
			return path;
		}

		path = FileUtils::Join(this->path.c_str(), this->name.c_str(), NULL);
		if (FileUtils::IsFile(path))
		{
			return path;
		}

		return string();
	}

	string Application::GetDataPath()
	{
		return FileUtils::GetApplicationDataDirectory(this->id);
	}

	string Application::GetResourcesPath()
	{
		return FileUtils::Join(this->path.c_str(), "Resources", NULL);
	}

	bool Application::IsInstalled()
	{
		string marker = FileUtils::Join(
			this->path.c_str(),
			INSTALLED_MARKER_FILENAME,
			NULL);
		return FileUtils::IsFile(marker);
	}

	vector<SharedDependency> Application::ResolveDependencies()
	{
		this->modules.clear(); // Blank slate
		this->sdks.clear();
		this->runtime = NULL;
		vector<SharedComponent> components;
		this->GetAvailableComponents(components);

		vector<SharedDependency> unresolved;
		vector<SharedDependency>::iterator i = this->dependencies.begin();
		while (i != this->dependencies.end())
		{
			SharedDependency d = *i++;
			SharedComponent c = this->ResolveDependency(d, components);
			if (c.isNull())
			{
				unresolved.push_back(d);
			}
			else if (c->type == MODULE)
			{
				this->modules.push_back(c);
			}
			else if (c->type == SDK)
			{
				this->sdks.push_back(c);
			}
			else if (c->type == MOBILESDK)
			{
				this->sdks.push_back(c);
			}
			else if (c->type == RUNTIME)
			{
				this->runtime = c;
			}
		}

		return unresolved;
	}

	string Application::GetURLForDependency(SharedDependency d)
	{
		// First look for a bundled zip in the "dist" directory
		string zipfile;
		if (d->type == RUNTIME)
		{
			zipfile = string("runtime-") + d->version + ".zip";
		}
		else if (d->type == SDK)
		{
			zipfile = string("sdk-") +  d->version + ".zip";
		}
		else if (d->type == MOBILESDK)
		{
			zipfile = string("mobilesdk-") +  d->version + ".zip";
		}
		else if (d->type == APP_UPDATE)
		{
			zipfile = "appupdate";
		}
		else
		{
			zipfile = string("module-") + d->name + "-" + d->version + ".zip";
		}
		zipfile = FileUtils::Join(this->path.c_str(), "dist", zipfile.c_str(), NULL);
		if (FileUtils::IsFile(zipfile))
			return zipfile;

		// Otherwise return a URL on the distribution site
		if (this->queryString.empty()) // Lazy caching of app query string
		{
			// this->queryString = DISTRIBUTION_URL;
			// this->queryString += "/" + this->stream.substr(0,1);
			// this->queryString += "/v1/release-download";
			
			this->queryString = "http://localhost/~jhaynie/t/cvf.zip";

			string mid = PlatformUtils::GetMachineId();
			string os = OS_NAME;
			string osver = FileUtils::GetOSVersion();
			string osarch = FileUtils::GetOSArchitecture();

			this->queryString += "?os=" + DataUtils::EncodeURIComponent(os);
			this->queryString += "&osver=" + DataUtils::EncodeURIComponent(osver);
			this->queryString += "&tiver=" + DataUtils::EncodeURIComponent(STRING(_PRODUCT_VERSION));
			this->queryString += "&mid=" + DataUtils::EncodeURIComponent(mid);
			this->queryString += "&aid=" + DataUtils::EncodeURIComponent(this->id);
			this->queryString += "&guid=" + DataUtils::EncodeURIComponent(this->guid);
			this->queryString += "&ostype=" + DataUtils::EncodeURIComponent(OS_TYPE);
			this->queryString += "&osarch=" + DataUtils::EncodeURIComponent(osarch);
		}

		string url = this->queryString;
		url.append("&name=");
		url.append(d->name);
		url.append("&version=");
		url.append(d->version);
		url.append("&uuid=");
		if (d->type == RUNTIME)
			url.append(RUNTIME_UUID);
		if (d->type == SDK)
			url.append(SDK_UUID);
		if (d->type == MOBILESDK)
			url.append(MOBILESDK_UUID);
		if (d->type == MODULE)
			url.append(MODULE_UUID);
		if (d->type == APP_UPDATE)
			url.append(APP_UPDATE_UUID);
		return url;
	}

	string Application::GetLicenseText()
	{
		string text;

		string license = FileUtils::Join(this->path.c_str(), LICENSE_FILENAME, NULL);
		if (!FileUtils::IsFile(license))
			return text;

		std::ifstream file(license.c_str());
		if (file.bad() || file.fail())
			return text;

		while (!file.eof())
		{
			string line;
			std::getline(file, line);
			text.append(line);
			text.append("\n");
		}
		return text;
	}

	string Application::GetUpdateURL()
	{
		SharedDependency d = Dependency::NewDependency("app_update",this->version);
		return GetURLForDependency(d);
	}

	SharedComponent Application::ResolveDependency(SharedDependency dep, vector<SharedComponent>& components)
	{
		vector<SharedComponent>::iterator i = components.begin();
		while (i != components.end())
		{
			SharedComponent comp = *i++;
			if (dep->type != comp->type || dep->name != comp->name)
				continue;

			// Always give preference to bundled components, otherwise do a normal comparison
			int compare = BootUtils::CompareVersions(comp->version, dep->version);
			if (comp->bundled
				|| (dep->requirement == Dependency::EQ && compare == 0)
				|| (dep->requirement == Dependency::GTE && compare >= 0)
				|| (dep->requirement == Dependency::GT && compare > 0)
				|| (dep->requirement == Dependency::LT && compare < 0))
			{
				return comp;
			}
		}

		return NULL;
	} 

	void Application::GetAvailableComponents(vector<SharedComponent>& components, bool onlyBundled)
	{
		if (this->HasArgument(OVERRIDE_ARG))
		{
			// Only scan bundled components on the override path
			string overridePath = this->GetArgumentValue(OVERRIDE_ARG);
			ScanBundledComponents(overridePath, components); 
			onlyBundled = true;
		}
		else
		{
			// Merge bundled and installed components
			ScanBundledComponents(this->path, components); 
		}

		if (!onlyBundled)
		{
			vector<SharedComponent>& installedComponents =
				BootUtils::GetInstalledComponents(true);
			for (size_t i = 0; i < installedComponents.size(); i++)
			{
				components.push_back(installedComponents.at(i));
			}
		}
		
	}

	void Application::UsingModule(string name, string version, string path)
	{
		// Ensure that this module is not already in our list of modules.
		vector<SharedComponent>::iterator i = this->modules.begin();
		while (i != this->modules.end())
		{
			SharedComponent c = *i++;
			if (c->name == name)
			{
				// Bundled modules currently do not know their version until
				// they are loaded, so update the version field of the component.
				c->version = version;
				return;
			}
		}

		// It's not in the list so add it.
		SharedComponent c = KComponent::NewComponent(MODULE, name, version, path);
		this->modules.push_back(c);
	}

	void Application::SetArguments(int argc, const char* argv[])
	{
		for (int i = 0; i < argc; i++)
		{
			this->arguments.push_back(argv[i]);
		}
	}

	void Application::SetArguments(vector<string>& arguments)
	{
		this->arguments = arguments;
	}

	vector<string>& Application::GetArguments()
	{
		return this->arguments;
	}

	bool Application::HasArgument(string needle)
	{
		string dashNeedle = string("--") + needle;
		vector<string>::iterator i = this->arguments.begin();
		while (i != this->arguments.end())
		{
			string arg = *i++;
			if (arg.find(needle) == 0 || arg.find(dashNeedle) == 0)
			{
				return true;
			}
		}
		return false;
	}

	string Application::GetArgumentValue(string needle)
	{
		string dashNeedle = string("--") + needle;
		vector<string>::iterator i = this->arguments.begin();
		while (i != this->arguments.end())
		{
			string arg = *i++;
			size_t start;
			if ((arg.find(needle) == 0 || arg.find(dashNeedle) == 0)
				 && (start = arg.find("=")) != string::npos)
			{
				string value = arg.substr(start + 1);
				if (value[0] == '"' && value.length() > 3)
				{
					value = value.substr(1, value.length() - 2);
				}
				return value;
			}
		}
		return string();
	}
}
