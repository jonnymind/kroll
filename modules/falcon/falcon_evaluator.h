/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef FALCON_EVALUATOR_H_
#define FALCON_EVALUATOR_H_

#include <falcon/intcomp.h>
#include <falcon/vm.h>
#include <falcon/item.h>

namespace kroll
{
	class FalconEvaluator : public StaticBoundObject
	{
	public:
		FalconEvaluator();
		virtual ~FalconEvaluator();
		void Evaluate(const ValueList& args, SharedValue result);
		void CanEvaluate(const ValueList& args, SharedValue result);

		Falcon::CoreClass* GetKObjectClass() { return m_kobj_class->item().asClass(); }
		Falcon::CoreClass* GetKMethodClass() { return m_kmth_class->item().asClass(); }
		Falcon::CoreClass* GetKListClass() { return m_klist_class->item().asClass(); }
		
	private:
		Falcon::InteractiveCompiler* m_intcomp;
		KVMachine* m_vm;
		Falcon::ModuleLoader* m_loader;
		
		// not necessary, but useful for performance.
		Falcon::GarbageLock* m_kobj_class;
		Falcon::GarbageLock* m_kmth_class;
		Falcon::GarbageLock* m_klist_class;
	};
}

#endif

