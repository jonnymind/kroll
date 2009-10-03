/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
 
#include "falcon_module.h"

namespace kroll
{
	KVMachine::KVMachine()
	{
		// We know we're the only real virtual machine in the system,
		// so we can set ourself as the current running machine even
		// if not started.
		setCurrent();
	}

}
