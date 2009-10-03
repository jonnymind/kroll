/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

/** Kroll specialized Virtual Machine.
	This provides a bit of extra support to Kroll.
*/

#ifndef _FALCON_KVM_H_
#define _FALCON_KVM_H_

namespace kroll
{
	class KVMachine: public Falcon::VMachine
	{
	public:
		KVMachine();
	};

}

#endif /* _FALCON_KVM_H_ */
