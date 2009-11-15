/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_FALCON_LIST_H_
#define _K_FALCON_LIST_H_

namespace kroll
{

	class KFalconList : public KList
	{

	public:
		KFalconList( Falcon::CoreArray* ca );
		virtual ~KFalconList();

		virtual void Append(KValueRef value);
		virtual unsigned int Size();
		virtual KValueRef At(unsigned int index);
		virtual void SetAt(unsigned int index, KValueRef value);
		virtual bool Remove(unsigned int index);

		virtual void Set(const char *name, KValueRef value);
		virtual KValueRef Get(const char *name);
		virtual SharedStringList GetPropertyNames();

		Falcon::CoreArray* ToFalcon() { return m_ca; }

	public:
		Falcon::CoreArray* m_ca;
		Falcon::GarbageLock* m_gl;
	};

}

#endif  /* _K_FALCON_LIST_H_ */

