/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <iostream>
#include <signal.h>
#include "javascript_module.h"
#include <Poco/Path.h>

namespace kroll
{
	KROLL_MODULE(JavascriptModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));

	JavascriptModule* JavascriptModule::instance = NULL;
	void JavascriptModule::Initialize()
	{
		JavascriptModule::instance = this;
		host->AddModuleProvider(this);

		SharedKObject global = this->host->GetGlobalObject();
		this->binding = new StaticBoundObject();
		global->Set("Javascript", Value::NewObject(this->binding));

		SharedKMethod evaluator = new JavascriptEvaluator();
		/**
		 * @tiapi(method=True,name=Javascript.evaluate,since=0.5) Evaluates a string as javascript code
		 * @tiarg(for=Javascript.evaluate,name=code,type=string) javascript code
		 * @tiarg(for=Javascript.evaluate,name=scope,type=object) global variable scope
		 * @tiresult(for=Javascript.evaluate,type=object) the result of the evaluation
		 */
		this->binding->Set("evaluate", Value::NewMethod(evaluator));

		SharedKMethod evaluateAsync = new JavascriptWorker(this->host);
		/**
		 * @tiapi(method=True,name=Javascript.evaluateInNewContext,since=0.5) Evaluates a piece of javascript code in a new JS context
		 * @tiarg(for=Javascript.evaluateInNewContext,name=code,type=any) javascript function or string URL
		 * @tiarg(for=Javascript.evaluateInNewContext,name=module_properties,type=object) top-level module properties
		 * @tiarg(for=Javascript.evaluateInNewContext,name=global_properties,type=object) top-level global properties
		 * @tiresult(for=Javascript.evaluateInNewContext,type=object) the result of the evaluation
		 */
		this->binding->Set("evaluateInNewContext", Value::NewMethod(evaluateAsync));
	}

	void JavascriptModule::Stop()
	{
		binding = NULL;
		JavascriptModule::instance = NULL;
	}

	const static std::string js_suffix = "module.js";
	bool JavascriptModule::IsModule(std::string& path)
	{
		int plength = path.length();
		int slength = js_suffix.length();
		if (path.length() > js_suffix.length())
		{
			return (path.substr(plength - slength) == js_suffix);
		}
		else
		{
			return false;
		}
	}

	Module* JavascriptModule::CreateModule(std::string& path)
	{
		Poco::Path p(path);
		std::string basename = p.getBaseName();
		std::string name = basename.substr(0,basename.length()-js_suffix.length()+3);
		std::string moduledir = path.substr(0,path.length()-basename.length()-3);

		Logger *logger = Logger::Get("Javascript");
		logger->Info("Loading JS path=%s", path.c_str());

		JavascriptModuleInstance* instance = new JavascriptModuleInstance(this->host, path, moduledir, name);
		return instance;
	}

}
