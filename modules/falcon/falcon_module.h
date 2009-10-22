/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _FALCON_MODULE_H
#define _FALCON_MODULE_H

#include <kroll/kroll.h>
#include <falcon/engine.h>

#include "falcon_api.h"
#include "falcon_kvm.h"
#include "falcon_evaluator.h"
#include "falcon_glue_module.h"
#include "k_falcon_list.h"
#include "k_falcon_object.h"
#include "k_falcon_method.h"
#include "k_falcon_func.h"
#include "k_falcon_dict.h"
#include "falcon_utils.h"
#include "falcon_module_instance.h"
#include "falcon_classes.h"

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

		static FalconEvaluator* Evaluator()
		{
			return evaluator_;
		}
		
	private:
		SharedKObject binding;
		static FalconModule *instance_;
		static FalconEvaluator *evaluator_;
		DISALLOW_EVIL_CONSTRUCTORS(FalconModule);
	};
}

#endif
