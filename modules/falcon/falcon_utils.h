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
		static KValueRef ToKrollValue(const Falcon::Item& value);

		/** Using an item passed by ref, we can often spare a useless copy operation. */
		static void ToFalconItem( KValueRef value, Falcon::Item& item );

			/** Throws a ValueError extracting and dereferencing the Falcon Error. */
		static void ManageError( Falcon::Error* e );
	};
}

#endif
