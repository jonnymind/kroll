/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_FALCON_DICT_H_
#define _K_FALCON_DICT_H_

namespace kroll {

/** Kroll-side falcon dictionary reflection.

	This Kroll class wrap a generic Falcon::CoreDictionary
	and exposes to Kroll the falcon object functionalities.

	It also consider blessed dictionaries and returns a method
	if getting a function from them.
*/
class KFalconDict : public KObject {
public:
	KFalconDict( Falcon::CoreDict* co );
	virtual ~KFalconDict();

	virtual void Set(const char *name, KValueRef value);
	virtual KValueRef Get(const char *name);
	virtual SharedStringList GetPropertyNames();

	/** Returns the Falcon Object associated with this core object. */
	Falcon::CoreDict* ToFalcon() const
	{
		return m_cd;
	}

	/**
	 * Determine if the given Falcon object equals this one
	 * by comparing these objects's identity e.g. equals?()
	 *  @param other the object to test
	 *  @returns true if objects have reference equality, false otherwise
	 */
	virtual bool Equals(KObjectRef other);

private:
	Falcon::CoreDict* m_cd;
	Falcon::GarbageLock* m_gl;
};

}

#endif /* _K_FALCON_DICT_H_ */
