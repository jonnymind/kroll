/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "php_module.h"
#include <sstream>
 
namespace kroll
{
	PHPEvaluator::PHPEvaluator()
		: StaticBoundObject("PHPEvaluator")
	{
		/**
		 * @tiapi(method=True,name=PHP.canEvaluate,since=0.7)
		 * @tiarg[String, mimeType] Code mime type
		 * @tiresult[bool] whether or not the mimetype is understood by PHP
		 */
		SetMethod("canEvaluate", &PHPEvaluator::CanEvaluate);

		/**
		 * @tiapi(method=True,name=PHP.evaluate,since=0.7) Evaluates a string as PHP code
		 * @tiarg[String, mimeType] Code mime type (normally "text/php")
		 * @tiarg[String, name] name of the script source
		 * @tiarg[String, code] PHP script code
		 * @tiarg[Object, scope] global variable scope
		 * @tiresult[Any] result of the evaluation
		 */
		SetMethod("evaluate", &PHPEvaluator::Evaluate);

		/**
		 * @tiapi(method=True,name=PHP.canPreprocess,since=0.7)
		 * @tiarg[String, url] URL to preprocess
		 * @tiresult[bool] whether or not the mimetype is understood by PHP
		 */
		SetMethod("canPreprocess", &PHPEvaluator::CanPreprocess);

		/**
		 * @tiapi(method=True,name=PHP.preprocess,since=0.7)
		 * Runs a string and URL through preprocessing
		 * @tiarg[String, url] URL used to load this resource
		 * @tiarg[Object, scope] Global variables to bind for PHP
		 * @tiresult[String] result of the evaluation
		 */
		SetMethod("preprocess", &PHPEvaluator::Preprocess);
	}

	void PHPEvaluator::CanEvaluate(const ValueList& args, SharedValue result)
	{
		args.VerifyException("canEvaluate", "s");
		
		result->SetBool(false);
		std::string mimeType = args.GetString(0);
		if (mimeType == "text/php")
		{
			result->SetBool(true);
		}
	}

	static std::string GetContextId(SharedKObject global)
	{
		std::string contextId(global->GetString("__php_module_id__"));
		if (contextId.empty())
		{
			static int nextId = 0;
			contextId.append("__kroll__namespace__");
			contextId.append(KList::IntToChars(++nextId));
			global->SetString("__php_module_id__", contextId);
		}

		return contextId;
	}

	void PHPEvaluator::Evaluate(const ValueList& args, SharedValue result)
	{
		static Poco::Mutex evaluatorMutex;
		Poco::Mutex::ScopedLock evaluatorLock(evaluatorMutex);

		args.VerifyException("evaluate", "s s s o");

		TSRMLS_FETCH();
		std::string mimeType = args.GetString(0);
		std::string name = args.GetString(1);
		std::string code = args.GetString(2);
		SharedKObject windowGlobal = args.GetObject(3);
		SharedValue kv = Value::Undefined;

		// Contexts must be the same for runs with the same global object.
		std::string contextId(GetContextId(windowGlobal));

		// Each function we create must have unique name though.
		static int nextFunctionId = 0;
		nextFunctionId++;

		std::ostringstream codeString;
		codeString << "namespace " << contextId << " {\n";
		codeString << code << "\n";
		codeString << "  $__fns = get_defined_functions();\n";
		codeString << "  if (array_key_exists(\"user\", $__fns)) {\n";
		codeString << "   foreach($__fns[\"user\"] as $fname) {\n";
		codeString << "    if (!$window->$fname) {";
		codeString << "      krollAddFunction($window, $fname);\n";
		codeString << "    }\n";
		codeString << "   }\n";
		codeString << "  }\n";
		codeString << "}\n";
		printf("%s\n", codeString.str().c_str());

		// This seems to be needed to make PHP actually give us errors
		// at parse/compile time -- see: main/main.c line 969
		PG(during_request_startup) = 0;

		SharedKObject previousGlobal(PHPUtils::GetCurrentGlobalObject());
		PHPUtils::SwapGlobalObject(windowGlobal, &EG(symbol_table) TSRMLS_CC);

		zend_first_try
		{
			zend_eval_string((char *) codeString.str().c_str(), NULL, 
				(char *) name.c_str() TSRMLS_CC);
		}
		zend_catch
		{
			Logger::Get("PHP")->Error("Evaluation of script failed");
		}
		zend_end_try();

		PHPUtils::SwapGlobalObject(previousGlobal, &EG(symbol_table) TSRMLS_CC);

		result->SetValue(kv);
	}

	void PHPEvaluator::CanPreprocess(const ValueList& args, SharedValue result)
	{
		args.VerifyException("canPreprocess", "s");

		std::string url = args.GetString(0);
		result->SetBool(false);
		if (Script::HasExtension(url.c_str(), "php"))
		{
			result->SetBool(true);
		}
	}

	void PHPEvaluator::FillServerVars(Poco::URI& uri, SharedKObject scope TSRMLS_DC)
	{
		// Fill $_SERVER with HTTP headers
		zval *SERVER;
		array_init(SERVER);
		
		//if (zend_hash_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), (void**)&SERVER) == SUCCESS)
		//{
		if (scope->HasProperty("httpHeaders"))
		{
			SharedStringList headerNames = scope->GetObject("httpHeaders")->GetPropertyNames();
			for (size_t i = 0; i < headerNames->size(); i++)
			{
				//zval *headerValue;
				const char *headerName = headerNames->at(i)->c_str();
				const char *headerValue = scope->GetObject("httpHeaders")->
					GetString(headerName).c_str();
				
				//ALLOC_INIT_ZVAL(headerValue);
				//ZVAL_STRING(headerValue, (char*)headers->GetString(headerName).c_str(), 1);
				
				add_assoc_stringl(SERVER, (char *) headerName, (char *) headerValue, strlen(headerValue), 1);
				//zend_hash_add(Z_ARRVAL_P(SERVER), (char*)headerName, strlen(headerName)+1, &headerValue, sizeof(zval*), NULL);
				//ZEND_SET_SYMBOL(Z_ARRVAL_P(SERVER), (char*)headerName, headerValue);
			}
			ZEND_SET_SYMBOL(&EG(symbol_table), (char *)"_SERVER", SERVER);
		}
		//}
		
		// Fill $_GET with query string parameters
		zval *GET;
		if (zend_hash_find(&EG(symbol_table), "_GET", sizeof("_GET"), (void**)&GET) == SUCCESS)
		{
			std::string queryString = uri.getQuery();
			Poco::StringTokenizer tokens(uri.getQuery(), "&=");
			for (Poco::StringTokenizer::Iterator iter = tokens.begin();
				iter != tokens.end(); iter++)
			{
				std::string key = *iter;
				std::string value = *(++iter);
				
				zval *val;
				ALLOC_INIT_ZVAL(val);
				ZVAL_STRING(val, (char*)value.c_str(), 1);
				zend_hash_add(Z_ARRVAL_P(GET), (char*)key.c_str(), key.size()+1, &val, sizeof(zval*), NULL);
			}
		}
		
		// TODO: Fill $_POST, $_REQUEST
	}
	
	void PHPEvaluator::Preprocess(const ValueList& args, SharedValue result)
	{
		args.VerifyException("preprocess", "s o");
		
		std::string url = args.GetString(0);
		Logger::Get("PHP")->Debug("preprocessing php => %s", url.c_str());
		
		Poco::URI uri(url);
		std::string path = URLUtils::URLToPath(url);
		
		SharedKObject scope = args.GetObject(1);
		TSRMLS_FETCH();

		PHPModule::SetBuffering(true);

		// These variables are normally initialized by php_module_startup
		// but we do not call that function, so we manually initialize.
		PG(header_is_being_sent) = 0;
		SG(request_info).headers_only = 0;
		SG(request_info).argv0 = NULL;
		SG(request_info).argc= 0;
		SG(request_info).argv= (char **) NULL;
		php_request_startup(TSRMLS_C);

		// This seems to be needed to make PHP actually give  us errors
		// at parse/compile time -- see: main/main.c line 969
		PG(during_request_startup) = 0;

		//FillServerVars(uri, scope TSRMLS_CC);
		zend_file_handle script;
		script.type = ZEND_HANDLE_FP;
		script.filename = (char*)path.c_str();
		script.opened_path = NULL;
		script.free_filename = 0;
		script.handle.fp = fopen(script.filename, "rb");

		zend_first_try
		{
			php_execute_script(&script TSRMLS_CC);
			
		}
		zend_catch
		{

		}
		zend_end_try();

		std::string output(PHPModule::GetBuffer().str());
		SharedKObject o = new StaticBoundObject();
		o->SetObject("data", new Blob(output.c_str(), output.size(), true));
		o->SetString("mimeType", PHPModule::GetMimeType().c_str());
		result->SetObject(o);

		PHPModule::SetBuffering(false);
	}
}
