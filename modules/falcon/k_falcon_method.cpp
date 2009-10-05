/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "falcon_module.h"

namespace kroll
{
	KFalconMethod::KFalconMethod( const Falcon::Item& fmethod ):
			KMethod( "KFalconMethod" ),
			m_glMethod( 0 )
	{
		if ( ! fmethod.isMethod() )
			throw ValueException::FromFormat(
				"Given item is not a Falcon Method, but an Item of type %d",
				fmethod.type() );
			
		m_glMethod = new Falcon::GarbageLock( fmethod );
	}
		
	KFalconMethod::~KFalconMethod()
	{
		delete m_glMethod;
	}
	
	SharedValue KFalconMethod::Call(const ValueList& args)
	{
		//TODO -- get the evaluator VM
		Falcon::VMachine* vm = Falcon::VMachine::getCurrent();

		size_t count = args.size();
		for ( size_t i = 0; i < count; ++i )
		{
			Falcon::Item param;
			FalconUtils::ToFalconItem( args[ i ], param );
			vm->pushParameter( param );
		}

		try {
			vm->callItem( m_glMethod->item(), count );
		}
		catch( Falcon::Error *e )
		{
			FalconUtils::ManageError( e );
		}

		return FalconUtils::ToKrollValue( vm->regA() );
	}
	
	void KFalconMethod::Set(const char *name, SharedValue value)
	{
		try {
			Falcon::Item itm;
			FalconUtils::ToFalconItem( value, itm );
			m_glMethod->item().asMethodItem().setProperty( name, itm );
		}
		catch( Falcon::Error* e )
		{
			FalconUtils::ManageError( e );
		}
	}

	
	SharedValue KFalconMethod::Get(const char *name)
	{
		Falcon::Item itm;
		
		try {
			m_glMethod->item().asMethodItem().getProperty( name, itm );
		}
		catch( Falcon::Error* e )
		{
			FalconUtils::ManageError( e );
		}
		
		return FalconUtils::ToKrollValue( itm );
	}

	
	SharedStringList KFalconMethod::GetPropertyNames()
	{
		SharedStringList names(new StringList());
		Falcon::Item self = m_glMethod->item().asMethodItem();

		// if this is an object or a blessed dictionary, its properties are self-declared.
		if( self.isObject() )
		{
			Falcon::CoreObject* co = self.asObjectSafe();
			const Falcon::PropertyTable& props = co->generator()->properties();

			for ( unsigned int i = 0; i < props.added(); ++i )
			{
				Falcon::AutoCString pname( *props.getKey( i ) );
				names->push_back( new std::string(pname.c_str()) );
			}
		}
		else if( self.isDict() && self.asDict()->isBlessed() )
		{
			Falcon::CoreDict* cd = self.asDict();
			Falcon::Iterator iter( &cd->items() );
			while( iter.hasCurrent() )
			{
				if (iter.getCurrentKey().isString() )
				{
					Falcon::AutoCString pname( *iter.getCurrentKey().asString() );
					names->push_back( new std::string(pname.c_str()) );
				}
			}
		}
		else
		{
			// otherwise, the properties are those of the metaclass.
			Falcon::CoreClass *cc = Falcon::VMachine::getCurrent()->getMetaClass( self.type() );
			const Falcon::PropertyTable& props = cc->properties();

			for ( unsigned int i = 0; i < props.added(); ++i )
			{
				Falcon::AutoCString pname( *props.getKey( i ) );
				names->push_back( new std::string(pname.c_str()) );
			}
		}

		return names;
	}

}

