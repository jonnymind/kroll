/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "falcon_module.h"

#include <cassert>

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
		m_vm = new KVMachine;

		// Put in the standard module.
		m_vm->link( Falcon::core_module_init() );

		// Put in the Kroll-glue module
		m_vm->link( Falcon::krollGlueModule() );

		// get our KObject class instance, so that we can use it later on.
		Falcon::Item* kobj_cls= m_vm->findWKI( FALCON_KOBJECT_CLASS_NAME );
		assert( kobj_cls != 0 );
		m_kobj_class = new Falcon::GarbageLock( *kobj_cls );

		Falcon::Item* kmth_cls= m_vm->findWKI( FALCON_KMETHOD_CLASS_NAME );
		assert( kmth_cls != 0 );
		m_kmth_class = new Falcon::GarbageLock( *kmth_cls );

		Falcon::Item* klist_cls= m_vm->findWKI( FALCON_KLIST_CLASS_NAME );
		assert( klist_cls != 0 );
		m_klist_class = new Falcon::GarbageLock( *klist_cls );
		
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

		delete m_kobj_class;
		delete m_kmth_class;
		delete m_klist_class;
		
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
		SharedKObject windowGlobal = args.GetObject(3);

		Logger *logger = Logger::Get("Falcon");
		// get the global window object
		// TODO: cache it globally
		Falcon::Item* i_window = m_vm->findGlobalItem( "window" );

		// turn into a falcon item
		FalconUtils::ToFalconItem( Value::NewObject(windowGlobal), *i_window );
		logger->Debug( "Running falcons script \"%s\"", name );
		try
		{
			m_intcomp->compileNext( code.c_str() );
		}
		catch( Falcon::Error* e )
		{
			FalconUtils::ManageError( e );
		}
		
		// TODO: return value from evaluated code goes here
		result->SetNull();
	}
}

