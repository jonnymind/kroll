/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef FALCON_EVALUATOR_H_
#define FALCON_EVALUATOR_H_

namespace kroll
{
	class FalconEvaluator : public StaticBoundObject
	{
	public:
		FalconEvaluator();
		void Evaluate(const ValueList& args, SharedValue result);
		void CanEvaluate(const ValueList& args, SharedValue result);
	};
}

#endif

