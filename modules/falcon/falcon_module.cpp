/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "falcon_module.h"

namespace kroll
{
	KROLL_MODULE(FalconModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));

	FalconModule* FalconModule::instance_ = NULL;

	void FalconModule::Initialize()
	{
		FalconModule::instance_ = this;

		// TODO: initialize vm here

		this->InitializeBinding();
		host->AddModuleProvider(this);
	}

	void FalconModule::Stop()
	{
		// TODO: do VM shutdown / cleanup here
	}

	void FalconModule::InitializeBinding()
	{
		SharedKObject global = this->host->GetGlobalObject();
		this->binding = new FalconEvaluator();
		global->Set("Falcon", Value::NewObject(this->binding));
		Script::GetInstance()->AddScriptEvaluator(this->binding);
	}

	const static std::string falcon_suffix = "module.fal";

	bool FalconModule::IsModule(std::string& path)
	{
		// TODO: re-enable this once we have module loading done
		return false;
		//return (path.substr(path.length()-falcon_suffix.length()) == falcon_suffix);
	}

	Module* FalconModule::CreateModule(std::string& path)
	{
		// TODO: falcon module loading (this can wait until later)
		return 0;
	}

}

