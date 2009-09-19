/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _FALCON_MODULE_INSTANCE_H
#define _FALCON_MODULE_INSTANCE_H

#include "falcon_module.h"

namespace kroll
{
	class FalconModuleInstance : public Module
	{
	public:
		FalconModuleInstance(Host *host, std::string path, std::string dir, std::string name);
	protected:
		virtual ~FalconModuleInstance();
	public:
		void Initialize ();
		void Destroy ();
	private:
		std::string path;
		DISALLOW_EVIL_CONSTRUCTORS(FalconModuleInstance);
	};
}

#endif
