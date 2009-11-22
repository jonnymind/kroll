/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "falcon_module.h"

namespace kroll
{

	KFalconList::KFalconList( Falcon::CoreArray* ca ):
		KList( "Falcon.KFalconList" ),
		m_ca( ca )
	{
		m_gl = new Falcon::GarbageLock( Falcon::Item(ca) );
	}
	
	KFalconList::~KFalconList()
	{
		delete m_gl;
	}

	void KFalconList::Append(KValueRef value)
	{
		Falcon::Item item;
		FalconUtils::ToFalconItem( value, item );
		m_ca->append( item );
	}
	
	unsigned int KFalconList::Size()
	{
		return m_ca->length();
	}
	
	KValueRef KFalconList::At(unsigned int index)
	{
		if ( m_ca->length() <= index )
		{
			return Value::Undefined;
		}
		return FalconUtils::ToKrollValue( m_ca->at( index ) );
	}


	void KFalconList::Set(const char *name, KValueRef value)
	{
		// Check for integer value as name
		int index = -1;
		if (KList::IsInt(name) && ((index = atoi(name)) >= 0))
		{
			this->SetAt((unsigned int) index, value);
		}
		else
		{
			try
			{
				Falcon::Item ivalue;
				FalconUtils::ToFalconItem( value, ivalue );
				this->m_ca->writeProperty(name, ivalue);
			}
			catch( Falcon::Error* e )
			{
				e->decref();
			}
		}
	}

	void KFalconList::SetAt(unsigned int index, KValueRef value)
	{
		if ( index >= m_ca->length() )
			m_ca->resize( index+1 );
		
		FalconUtils::ToFalconItem( value, m_ca->at( index ) );
	}

	KValueRef KFalconList::Get(const char *name)
	{
		if (KList::IsInt(name))
		{
			unsigned int index = (unsigned int) atoi(name);
			if (index >= 0)
				return this->At(index);
		}

		Falcon::String sname(name);
		
		if( sname == "length" )
			return  Value::NewDouble( (double) this->m_ca->length() );
		
		try
		{
			Falcon::Item prop;
			this->m_ca->readProperty(name, prop);
			return FalconUtils::ToKrollValue( prop );
		}
		catch( Falcon::Error* e )
		{
			e->decref();
		}
		
		return Value::Undefined;
	}

	SharedStringList KFalconList::GetPropertyNames()
	{
		SharedStringList property_names; // = object->GetPropertyNames();
		for (size_t i = 0; i < this->Size(); i++)
		{
			std::string name = KList::IntToChars(i);
			property_names->push_back(new std::string(name));
		}

		property_names->push_back( new std::string("length") );

		return property_names;
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

	bool KFalconList::Equals(KObjectRef other)
	{
		AutoPtr<KFalconList> lOther = other.cast<KFalconList>();

		// This is not a Python object
		if (lOther.isNull())
			return false;

		return lOther->ToFalcon() == this->ToFalcon();
	}

}
