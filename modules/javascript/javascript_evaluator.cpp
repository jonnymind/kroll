/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "javascript_evaluator.h"

SharedValue JavascriptEvaluator::Call(const ValueList& args)
{
	if (args.size() != 3
		|| !args.at(1)->IsString())
	{
		return Value::Undefined;
	}

	SharedValue code = args.at(1);
	if (args.at(2)->IsObject())
	{
		SharedKObject window_global = args.at(2)->ToObject();

		SharedValue evalMethod = window_global->Get("eval");
		if (evalMethod->IsMethod())
		{
			SharedKMethod eval = evalMethod->ToMethod();
			ValueList evalArgs;
			evalArgs.push_back(code);
			return eval->Call(evalArgs);
		}
		else
		{
			throw ValueException::FromString("no eval method found in window scope");
		}
	}
	else if (args.at(2)->IsVoidPtr())
	{
		// in this case, we're passing in a context and a specific type
		JSContextRef ctx = (JSContextRef)args.at(2)->ToVoidPtr();
		std::string type = args.at(0)->ToString();
		if (type == "text/url")
		{
			return KJSUtil::EvaluateFile(ctx,(char*)args.at(1)->ToString());
		}
		else
		{
			return KJSUtil::Evaluate(ctx,(char*)args.at(1)->ToString());
		}
	}
	
	return Value::Undefined;
}

void JavascriptEvaluator::Set(const char *name, SharedValue value)
{
}

SharedValue JavascriptEvaluator::Get(const char *name)
{
	return Value::Undefined;
}

SharedStringList JavascriptEvaluator::GetPropertyNames()
{
	return SharedStringList();
}
