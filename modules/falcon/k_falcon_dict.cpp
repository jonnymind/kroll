/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "falcon_module.h"

namespace kroll {

	KFalconDict::KFalconDict( Falcon::CoreDict* cd ):
		m_cd( cd )
	{
		m_gl = new Falcon::GarbageLock( Falcon::Item(cd) );
	}

	KFalconDict::~KFalconDict()
	{
		delete m_gl;
	}

	void KFalconDict::Set(const char *name, SharedValue value)
	{
		try
		{
			Falcon::Item item;
			FalconUtils::ToFalconItem( value, item );
			m_cd->writeProperty( name, item );
		}
		catch( Falcon::Error *e )
		{
			FalconUtils::ManageError( e );
		}
	}

	SharedValue KFalconDict::Get(const char *name)
	{
		Falcon::Item item;
		try
		{
			m_cd->readProperty( name, item );
		}
		catch( Falcon::Error *e )
		{
			FalconUtils::ManageError( e );
		}

		// putting the return out of the catch to make the compiler happy:
		// ManageError always throws.
		return FalconUtils::ToKrollValue( item );
	}

	SharedStringList KFalconDict::GetPropertyNames()
	{
		SharedStringList names(new StringList());
		Falcon::Iterator iter( &m_cd->items() );
		while( iter.hasCurrent() )
		{
			if( iter.getCurrentKey().isString() )
			{
				Falcon::AutoCString pname( *iter.getCurrentKey().asString() );
				names->push_back( new std::string(pname.c_str()) );
			}
			iter.next();
		}

		return names;
	}


	bool KFalconDict::Equals( SharedKObject other )
	{
		AutoPtr<KFalconDict> falconOther = other.cast<KFalconDict>();
		if (falconOther.isNull())
			return false;

		return this->ToFalcon() == falconOther->ToFalcon();
	}

}

