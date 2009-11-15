/** Appcelerator Kroll - licensed under the Apache Public License 2 see LICENSE
 * in the root folder for details on the license.  Copyright (c) 2008
 * Appcelerator, Inc. All Rights Reserved.
 */
#include <signal.h>
#include "ruby_module.h"
#include <Poco/Path.h>

namespace kroll
{
	KROLL_MODULE(RubyModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));

	RubyModule* RubyModule::instance_ = NULL;

	void RubyModule::Initialize()
	{
		RubyModule::instance_ = this;

		ruby_init();
		ruby_init_loadpath();

		string modulePath = this->GetPath();

		// Add the application directoy to the Ruby include path so
		// that includes work in a intuitive way for application developers.
		ruby_incpush(host->GetApplication()->GetResourcesPath().c_str());

		this->InitializeBinding();

		host->AddModuleProvider(this);
	}

	void RubyModule::Stop()
	{
		KObjectRef global = this->host->GetGlobalObject();
		global->Set("Ruby", Value::Undefined);
		Script::GetInstance()->RemoveScriptEvaluator(this->binding);
		this->binding = NULL;
		RubyModule::instance_ = NULL;

		ruby_cleanup(0);
	}

	void RubyModule::InitializeBinding()
	{
		// Expose the Ruby evaluator into Kroll
		KObjectRef global = this->host->GetGlobalObject();
		this->binding = new RubyEvaluator();
		global->Set("Ruby", Value::NewObject(binding));
		Script::GetInstance()->AddScriptEvaluator(this->binding);
		
		// Bind the API global constant
		VALUE ruby_api_val = RubyUtils::KObjectToRubyValue(Value::NewObject(global));
		rb_define_global_const(PRODUCT_NAME, ruby_api_val);
	}


	const static std::string ruby_suffix = "module.rb";
	bool RubyModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-ruby_suffix.length()) == ruby_suffix);
	}

	Module* RubyModule::CreateModule(std::string& path)
	{
		rb_load_file(path.c_str());
		ruby_exec();

//		ruby_cleanup();  <-- at some point we need to call?


		Poco::Path p(path);
		std::string basename = p.getBaseName();
		std::string name = basename.substr(0,basename.length()-ruby_suffix.length()+3);
		std::string moduledir = path.substr(0,path.length()-basename.length()-3);

		Logger *logger = Logger::Get("Ruby");
		logger->Info("Loading Ruby path=%s", path.c_str());

		return new RubyModuleInstance(host, path, moduledir, name);
	}

}
