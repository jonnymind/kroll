/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "falcon_module.h"
#include <falcon/corefunc.h>

namespace kroll
{

	KFalconFunc::KFalconFunc( const Falcon::Item& ffunc ):
			KMethod( "KFalconFunc" ),
			m_glFunc( 0 )
	{
		if ( ! ffunc.isFunction() )
			throw ValueException::FromFormat(
				"Given item is not a Falcon Function, but an Item of type %d",
				ffunc.type() );

		m_glFunc = new Falcon::GarbageLock( ffunc );
	}

	KFalconFunc::~KFalconFunc()
	{
		delete m_glFunc;
	}

	KValueRef KFalconFunc::Call(const ValueList& args)
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
			vm->callItem( m_glFunc->item(), count );
		}
		catch( Falcon::Error *e )
		{
			FalconUtils::ManageError( e );
		}

		return FalconUtils::ToKrollValue( vm->regA() );
	}

	void KFalconFunc::Set(const char *name, KValueRef value)
	{
		throw ValueException::FromString( "Cannot set property on Falcon Function object." );
	}


	KValueRef KFalconFunc::Get(const char *name)
	{
		if( std::string( name ) == "name" )
		{
			Falcon::AutoCString cname( m_glFunc->item().asFunction()->name() );
			return Value::NewString( cname.c_str() );
		}
		return Value::NewNull();
	}


	SharedStringList KFalconFunc::GetPropertyNames()
	{
		SharedStringList names(new StringList());
		names->push_back( SharedString(new std::string("name")) );
		return names;
	}
}
