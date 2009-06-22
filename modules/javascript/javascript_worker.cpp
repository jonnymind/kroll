/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "javascript_worker.h"

JavascriptWorker::JavascriptWorker(Host *host) : host(host)
{
}

SharedValue JavascriptWorker::Call(const ValueList& args)
{
	if (args.size()!=3)
	{
		throw ValueException::FromString("invalid arguments");
	}
	SharedKObject properties;
	SharedKObject global_properties;
	
	if (args.at(1)->IsObject())
	{
		properties = args.at(1)->ToObject();
	}
	if (args.at(2)->IsObject())
	{
		global_properties = args.at(2)->ToObject();
	}
	
	return kroll::KJSUtil::EvaluateInNewContext(this->host,properties,global_properties,args.at(0)->ToString());
}

void JavascriptWorker::Set(const char *name, SharedValue value)
{
}

SharedValue JavascriptWorker::Get(const char *name)
{
	return Value::Undefined;
}

SharedStringList JavascriptWorker::GetPropertyNames()
{
	return SharedStringList();
}
