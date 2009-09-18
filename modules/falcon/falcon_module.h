/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _FALCON_MODULE_H
#define _FALCON_MODULE_H

#include <kroll/kroll.h>

#include "falcon_api.h"
#include "falcon_evaluator.h"

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

#endif
