/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascript_module.h"
#include <iostream>
#include <fstream>
#include <string>

namespace kroll
{
	// TODO: Implement real method metadata and lifecycle events for
	// scripting language-based modules
	JavascriptModuleInstance::JavascriptModuleInstance(Host *host, std::string path, std::string dir, std::string name) :
		Module(host, dir.c_str(), name.c_str(), "0.1"), path(path)
	{
		try
		{
			this->Load();
			this->Run();
		}
		catch (ValueException& e)
		{
			SharedString ss = e.GetValue()->DisplayString();
			Logger *logger = Logger::Get("Javascript");
			logger->Error("Could not execute %s because %s", path.c_str(), (*ss).c_str());
		}

	}
	void JavascriptModuleInstance::Initialize () {}
	void JavascriptModuleInstance::Destroy () {}

	void JavascriptModuleInstance::Load()
	{
		this->code = "";
		std::ifstream js_file(this->path.c_str());
		if (!js_file.is_open())
		{
			throw ValueException::FromString("Could not read Javascript file");
		}

		std::string line;
		while (!js_file.eof() )
		{
			std::getline(js_file, line);
			this->code.append(line);
			this->code.append("\n");
		}
		js_file.close();
	}

	void JavascriptModuleInstance::Run()
	{
		KJSUtil::EvaluateInNewContext(host,SharedKObject(NULL),SharedKObject(NULL),this->code.c_str());
		
		// null it out so we don't hold a bunch of this in memory
		this->code = "";
	}

}

