/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_FALCON_OBJECT_H_
#define _K_FALCON_OBJECT_H_

namespace kroll {

class KFalconObject : public KObject {
public:
	KFalconObject( Falcon::CoreObject* co );
	virtual ~KFalconObject();

	virtual void Set(const char *name, SharedValue value);
	virtual SharedValue Get(const char *name);

	virtual SharedStringList GetPropertyNames();

	// TODO: Atm, falcon does not expose a method to have a VM independent object representation.
	//virtual SharedString DisplayString(int levels = 3);

	/** Returns the Falcon Object associated with this core object. */
	Falcon::CoreObject* ToFalcon() const
	{
		return m_co;
	}

	/**
	 * Determine if the given Falcon object equals this one
	 * by comparing these objects's identity e.g. equals?()
	 *  @param other the object to test
	 *  @returns true if objects have reference equality, false otherwise
	 */
	virtual bool Equals(SharedKObject other);

private:
	Falcon::CoreObject* m_co;
	Falcon::GarbageLock* m_gl;

	/** Throws a ValueError extracting and dereferencing the Falcon Error */
	void ManageError( Falcon::Error* e );
};

}

#endif /* _K_FALCON_OBJECT_H_ */
