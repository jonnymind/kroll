/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "falcon_module.h"

namespace Falcon
{

	Module* krollGlueModule()
	{
		Module *self = new Module();
		self->name( "kroll" );

		#define FALCON_DECLARE_MODULE self
		
		self->language( "en_US" );
		self->engineVersion( FALCON_VERSION_NUM );
		self->version( 0x000100 );

		return self;
	}

}

