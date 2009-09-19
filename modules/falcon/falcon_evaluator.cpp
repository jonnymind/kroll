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
		//const char *name = args.GetString(1).c_str();
		std::string code = args.GetString(2);
		//SharedKObject windowGlobal = args.GetObject(3);

		// TODO: evaluate the code here

		// TODO: return value from evaluated code goes here
		result->SetNull();
	}
}

