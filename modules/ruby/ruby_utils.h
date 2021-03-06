/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef RUBY_TYPES_H_
#define RUBY_TYPES_H_

#include <typeinfo>
#include "ruby_module.h"

namespace kroll
{
	class RubyUtils
	{
	public:
		static SharedValue ToKrollValue(VALUE value);
		static VALUE ToRubyValue(SharedValue value);
		static VALUE KObjectToRubyValue(SharedValue value);
		static VALUE KMethodToRubyValue(SharedValue value);
		static VALUE KListToRubyValue(SharedValue value);
		static bool KindOf(VALUE value, VALUE klass);

		static ValueException GetException();
		static VALUE GenericKMethodCall(SharedKMethod method, VALUE args);

	private:
		static VALUE KObjectClass;
		static VALUE KMethodClass;
		static VALUE KListClass;
		RubyUtils(){}
		~RubyUtils(){}
	};
}

#endif
