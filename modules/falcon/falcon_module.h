/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _FALCON_MODULE_H
#define _FALCON_MODULE_H

#if defined(OS_OSX) || defined(OS_LINUX)
#define EXPORT __attribute__((visibility("default")))
#define KROLL_FALCON_API EXPORT
#elif defined(OS_WIN32)
# ifdef KROLL_FALCON_API_EXPORT
#  define KROLL_FALCON_API __declspec(dllexport)
# else
#  define KROLL_FALCON_API __declspec(dllimport)
# endif
#endif


#include <kroll/base.h>
#include <string>
#include <string>
#include <vector>

#undef sleep
#undef close
#undef shutdown

#include <kroll/kroll.h>

#include <falcon/engine.h>

namespace kroll
{
	class FalconModule : public Module, public ModuleProvider
	{
		KROLL_MODULE_CLASS(FalconModule)

	public:
		virtual bool IsModule(std::string& path);
		virtual Module* CreateModule(std::string& path);
		void InitializeBinding();

		virtual const char * GetDescription() 
		{
			return "Falcon Module Loader";
		}
		Host* GetHost()
		{
			return host;
		}
		static FalconModule* Instance()
		{
			return instance_;
		}

	private:
		SharedKObject binding;
		static FalconModule *instance_;
		DISALLOW_EVIL_CONSTRUCTORS(FalconModule);
	};

}

#include "falcon_module_instance.h"

#endif
