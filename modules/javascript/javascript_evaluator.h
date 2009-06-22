/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef JAVASCRIPT_EVALUATOR_H_
#define JAVASCRIPT_EVALUATOR_H_

#include "javascript_module.h"

namespace kroll
{
	class JavascriptEvaluator : public KMethod
	{
	public:
		virtual SharedValue Call(const ValueList& args);
		virtual void Set(const char *, SharedValue);
		virtual SharedValue Get(const char *);
		virtual SharedStringList GetPropertyNames();
	};
}

#endif

