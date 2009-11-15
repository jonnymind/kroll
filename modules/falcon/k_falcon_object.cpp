/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "falcon_module.h"

namespace kroll {

	KFalconObject::KFalconObject( Falcon::CoreObject* co ):
		m_co( co )
	{
		m_gl = new Falcon::GarbageLock( Falcon::Item(co) );
	}
	
	KFalconObject::~KFalconObject()
	{
		delete m_gl;
	}

	void KFalconObject::Set(const char *name, KValueRef value)
	{
		try
		{
			Falcon::Item item;
			FalconUtils::ToFalconItem( value, item );
			m_co->writeProperty( name, item );
		}
		catch( Falcon::Error *e )
		{
			FalconUtils::ManageError( e );
		}
	}
	
	KValueRef KFalconObject::Get(const char *name)
	{
		Falcon::Item item;
		try
		{
			m_co->readProperty( name, item );
		}
		catch( Falcon::Error *e )
		{
			FalconUtils::ManageError( e );
		}
		
		// putting the return out of the catch to make the compiler happy:
		// ManageError always throws.
		return FalconUtils::ToKrollValue( item );
	}

	SharedStringList KFalconObject::GetPropertyNames()
	{
		SharedStringList names(new StringList());
		
		const Falcon::PropertyTable& props = m_co->generator()->properties();

		for ( unsigned int i = 0; i < props.added(); ++i )
		{
			Falcon::AutoCString pname( *props.getKey( i ) );
			names->push_back( new std::string(pname.c_str()) );
		}
		
		return names;
	}

	
	bool KFalconObject::Equals( KObjectRef other )
	{
		AutoPtr<KFalconObject> falconOther = other.cast<KFalconObject>();
		if (falconOther.isNull())
			return false;

		return this->ToFalcon() == falconOther->ToFalcon();
	}

}

