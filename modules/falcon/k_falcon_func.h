/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_FALCON_FUNC_H_
#define _K_FALCON_FUNC_H_

namespace kroll
{
	/** Falcon distinguishes between method and functions.
		It's not a great deal, just the KFalconMethod KMethod
		overloads are overloades so that they can access the underlying
		"self" item inside the method.

		This can't be done on Falcon Functions; we provide a "name"
		property just as a simple utility.
	*/
	class KFalconFunc : public KMethod
	{

	public:
		KFalconFunc( const Falcon::Item& fmethod );
		virtual ~KFalconFunc();

		virtual KValueRef Call(const ValueList& args);
		virtual void Set(const char *name, KValueRef value);
		virtual KValueRef Get(const char *name);
		virtual SharedStringList GetPropertyNames();

		Falcon::Item& ToFalcon() { return m_glFunc->item(); }
		const Falcon::Item& ToFalcon() const { return m_glFunc->item(); }

	public:
		Falcon::GarbageLock* m_glFunc;
	};

}

#endif  /* _K_FALCON_FUNC_H_ */
