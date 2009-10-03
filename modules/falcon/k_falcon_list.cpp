/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "falcon_module.h"

namespace kroll
{

	KFalconList::KFalconList( Falcon::CoreArray* ca ):
		KList( "KFalconList" ),
		m_ca( ca )
	{
		m_gl = new Falcon::GarbageLock( Falcon::Item(ca) );
	}
	
	KFalconList::~KFalconList()
	{
		delete m_gl;
	}

	void KFalconList::Append(SharedValue value)
	{
		Falcon::Item item;
		FalconUtils::ToFalconItem( value, item );
		m_ca->append( item );
	}
	
	unsigned int KFalconList::Size()
	{
		return m_ca->length();
	}
	
	SharedValue KFalconList::At(unsigned int index)
	{
		if ( m_ca->length() <= index )
		{
			throw ValueException::FromFormat( "KFalconList::At out of range( %d/%d )", index, m_ca->length() );
		}
		return FalconUtils::ToKrollValue( m_ca->at( index ) );
	}
	
	void KFalconList::SetAt(unsigned int index, SharedValue value)
	{
		if ( m_ca->length() <= index )
		{
			throw ValueException::FromFormat( "KFalconList::SetAt out of range( %d/%d )", index, m_ca->length() );
		}

		FalconUtils::ToFalconItem( value, m_ca->at( index ) );
	}
	
	bool KFalconList::Remove(unsigned int index)
	{
		if ( m_ca->length() <= index )
		{
			return false;
		}

		m_ca->remove( index );
		return true;
	}

	void KFalconList::Set(const char *name, SharedValue value)
	{
		// For now, nothing.
	}
	
	SharedValue KFalconList::Get(const char *name)
	{
		// For now nothing
		return Value::NewNull();
	}
	
	SharedStringList KFalconList::GetPropertyNames()
	{
		// For now nothing
		SharedStringList names(new StringList());
		return names;
	}


}
