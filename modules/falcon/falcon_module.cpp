/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <signal.h>
#include "falcon_module.h"
#include <Poco/Path.h>

namespace kroll
{
	KROLL_MODULE(FalconModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));

	FalconModule* FalconModule::instance_ = NULL;

	void FalconModule::Initialize()
	{
		FalconModule::instance_ = this;

		Falcon::Engine::Init();
		// The Falcon search path is an added path global for the whole engine.
		// It is also propagated to scripts on their request.
		Falcon::Engine::setSearchPath( host->GetApplication()->GetResourcesPath().c_str());

		this->InitializeBinding();
		host->AddModuleProvider(this);
	}

	void FalconModule::Stop()
	{
		SharedKObject global = this->host->GetGlobalObject();
		global->Set("Falcon", Value::Undefined);
		Script::GetInstance()->RemoveScriptEvaluator(this->binding);
		this->binding = NULL;
		FalconModule::instance_ = NULL;

		Falcon::Engine::Shutdown();
	}

	void FalconModule::InitializeBinding()
	{
	/*
		SharedKObject global = this->host->GetGlobalObject();
		this->binding = new FalconEvaluator();
		global->Set("Falcon", Value::NewObject(this->binding));
		Script::GetInstance()->AddScriptEvaluator(this->binding);
		*/
	}


	const static std::string falcon_suffix = "module.fal";

	bool FalconModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-falcon_suffix.length()) == falcon_suffix);
	}

	Module* FalconModule::CreateModule(std::string& path)
	{
		Logger *logger = Logger::Get("Falcon");

		// run the main code of the module -- why?
		try {
			Falcon::ModuleLoader ml(".");
			ml.addFalconPath();
			ml.addSearchPath( Falcon::Engine::getSearchPath() );

			Falcon::Runtime rt( &ml );
			rt.loadFile( path.c_str() );

			Falcon::VMachineWrapper vm;
			vm->link( &rt );
			vm->launch();
		}
		catch( Falcon::Error *errc )
		{
			Falcon::AutoCString err( errc->toString() );
			logger->Error("Error loading Falcon path=%s\n%s", path.c_str(), err.c_str() );
			errc->decref();
			return 0;
		}

		Poco::Path p( path );
		std::string basename = p.getBaseName();
		std::string name = basename.substr(0,basename.length()-falcon_suffix.length()+3);
		std::string moduledir = path.substr(0,path.length()-basename.length()-3);

		logger->Info("Loading Falcon path=%s", path.c_str());

		return new FalconModuleInstance(host, path, moduledir, name);
	}

}
