/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_FALCON_METHOD_H_
#define _K_FALCON_METHOD_H_

namespace kroll
{

	class KFalconMethod : public KMethod
	{

	public:
		KFalconMethod( const Falcon::Item& fmethod );
		virtual ~KFalconMethod();
		
		virtual SharedValue Call(const ValueList& args);
		virtual void Set(const char *name, SharedValue value);
		virtual SharedValue Get(const char *name);
		virtual SharedStringList GetPropertyNames();

		Falcon::Item& ToFalcon() { return m_glMethod->item(); }
		const Falcon::Item& ToFalcon() const { return m_glMethod->item(); }
		
	public:
		Falcon::GarbageLock* m_glMethod;
	};

}

#endif  /* _K_FALCON_METHOD_H_ */
