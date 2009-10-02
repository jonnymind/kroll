/**
* Appcelerator Kroll - licensed under the Apache Public License 2
* see LICENSE in the root folder for details on the license.
* Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
*/
#ifndef FALCON_TYPES_H_
#define FALCON_TYPES_H_

#include <typeinfo>
#include "falcon_module.h"

namespace kroll
{
	class FalconUtils
	{
	public:
		static SharedValue ToKrollValue(const Falcon::Item& value);

		/** Using an item passed by ref, we can often spare a useless copy operation. */
		static void ToFalconItem( const SharedValue& value, Falcon::Item& item );
	};
}

#endif
