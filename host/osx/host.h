/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _OSX_HOST_H
#define _OSX_HOST_H

#include <api/kroll.h>

namespace kroll
{
	class EXPORT OSXHost : public Host
	{
	public:
		OSXHost(int argc, const char **argv);

		virtual Module* CreateModule(std::string& path);
		const char* GetPlatform();
		const char* GetModuleSuffix();
		virtual bool IsMainThread();
		virtual KValueRef RunOnMainThread(KMethodRef method, KObjectRef thisObject,
			const ValueList& args, bool waitForCompletion=true);

	protected:
		virtual ~OSXHost();
		virtual bool RunLoop();
		virtual bool Start();
		virtual void Exit(int exitcode);
	};
}

extern "C"
{
	EXPORT int Execute(int argc,const char** argv);
}

#endif
