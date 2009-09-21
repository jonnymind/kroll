/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "falcon_module.h"

namespace kroll
{
	FalconEvaluator::FalconEvaluator()
		: StaticBoundObject("Falcon")
	{
		/**
		 * @tiapi(method=True,name=Falcon.canEvaluate,since=0.7)
		 * @tiarg[String, mimeType] Code mime type
		 * @tiresult[bool] whether or not the mimetype is understood by Falcon
		 */
		SetMethod("canEvaluate", &FalconEvaluator::CanEvaluate);
		
		/**
		 * @tiapi(method=True,name=Falcon.evaluate,since=0.2) Evaluates a string as Falcon code
		 * @tiarg[String, mimeType] Code mime type (normally "text/falcon")
		 * @tiarg[String, name] name of the script source
		 * @tiarg[String, code] Falcon script code
		 * @tiarg[Object, scope] global variable scope
		 * @tiresult[Any] result of the evaluation
		 */
		SetMethod("evaluate", &FalconEvaluator::Evaluate);

		// create the virtual machine.
		m_vm = new Falcon::VMachine;

		// Put in the standard module.
		m_vm->link( Falcon::core_module_init() );
		
		// Create a module loader using the default engine path settings
		m_loader = new Falcon::ModuleLoader( "." );
		m_loader->addSearchPath(Falcon::Engine::getSearchPath());
		m_loader->addFalconPath();
		
		// Create the compiler.
		m_intcomp = new Falcon::InteractiveCompiler( m_loader, m_vm );
	}


	FalconEvaluator::~FalconEvaluator()
	{
		Logger *logger = Logger::Get("Falcon");
		logger->Debug( "Terminating the Falcon Evaluator" );

		delete m_intcomp;
		delete m_loader;
		m_vm->finalize();
	}


	void FalconEvaluator::CanEvaluate(const ValueList& args, SharedValue result)
	{
		args.VerifyException("canEvaluate", "s");
		
		result->SetBool(false);
		std::string mimeType = args.GetString(0);
		if (mimeType == "text/falcon")
		{
			result->SetBool(true);
		}
	}
	
	void FalconEvaluator::Evaluate(const ValueList& args, SharedValue result)
	{
		args.VerifyException("evaluate", "s s s o");

		//const char *mimeType = args.GetString(0).c_str();
		const char *name = args.GetString(1).c_str();
		std::string code = args.GetString(2);
		//SharedKObject windowGlobal = args.GetObject(3);

		Logger *logger = Logger::Get("Falcon");
		
		try
		{
			logger->Debug( "Executing falcon script \"%s\":\n%s", name, code.c_str());
			m_intcomp->compileNext( code.c_str() );
		}
		catch( Falcon::Error* e )
		{
			Falcon::AutoCString err( e->toString() );
			logger->Error("Error in Falcon Script: %s", err.c_str() );
			e->decref();
		}
		
		
		// TODO: return value from evaluated code goes here
		result->SetNull();
	}
}

