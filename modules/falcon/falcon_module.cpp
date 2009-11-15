/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
 
#include "falcon_module.h"
#include <Poco/Path.h>

namespace kroll
{
	KROLL_MODULE(FalconModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));

	FalconModule* FalconModule::instance_ = NULL;
	FalconEvaluator* FalconModule::evaluator_ = NULL;
	
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
		Logger *logger = Logger::Get("Falcon");
		logger->Debug( "Stopping Falcon module" );

		
		KObjectRef global = this->host->GetGlobalObject();
		global->Set("Falcon", Value::Undefined);
		Script::GetInstance()->RemoveScriptEvaluator(this->binding);
		logger->Debug( "Binding count at cleanup: %d", this->binding->referenceCount() );
		this->binding->release();
		this->binding = NULL;
		FalconModule::instance_ = NULL;

		logger->Debug( "Performing last GC cleanup" );
		Falcon::memPool->performGC();
		logger->Debug( "Performing engine shutdown" );
		Falcon::Engine::Shutdown();
		logger->Debug( "Shutdown complete" );
	}

	void FalconModule::InitializeBinding()
	{
		KObjectRef global = this->host->GetGlobalObject();
		evaluator_ = new FalconEvaluator();
		this->binding = evaluator_;
		global->Set("Falcon", Value::NewObject(this->binding));
		Script::GetInstance()->AddScriptEvaluator(this->binding);
		Logger::Get("Falcon")->Debug( "Binding count at initializing: %d", this->binding->referenceCount() );
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
         // rethrows an exception.
         FalconUtils::ManageError( errc );
		}

		Poco::Path p( path );
		std::string basename = p.getBaseName();
		std::string name = basename.substr(0,basename.length()-falcon_suffix.length()+3);
		std::string moduledir = path.substr(0,path.length()-basename.length()-3);

		logger->Info("Loading Falcon path=%s", path.c_str());

		return new FalconModuleInstance(host, path, moduledir, name);
	}

}

