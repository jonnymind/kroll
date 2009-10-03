/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "falcon_module.h"

namespace Falcon
{

//=====================================================
// Falcon side of the KObject implementation
//
// This allows falcon script to access remote KObjects
// as wrapped in a falcon class.
//=====================================================

	class FKObject: public CoreObject
	{
	public:
		FKObject( const CoreClass* cls, kroll::SharedValue data ):
			CoreObject( cls ),
			m_data( data )
		{}

		FKObject( const FKObject& other ):
			CoreObject( other ),
			m_data( other.m_data )
		{}

		virtual ~FKObject() {}

		virtual bool hasProperty( const String &key ) const
		{
			AutoCString ckey( key );
			return m_data->ToObject()->HasProperty( ckey.c_str() );
		}

		virtual bool setProperty( const String &prop, const Item &value )
		{
			AutoCString ckey( prop );
			kroll::SharedValue v = kroll::FalconUtils::ToKrollValue( value );
			// In case of problems, this should fire an application specific exception.
			m_data->ToObject()->Set( ckey.c_str(), v );

			return true;
		}

		virtual bool getProperty( const String &key, Item &ret ) const
		{
			AutoCString ckey( key );
			// In case of problems, this should fire an application specific exception
			kroll::SharedValue v = m_data->ToObject()->Get( ckey.c_str() );
			kroll::FalconUtils::ToFalconItem( v, ret );
			return true;
		}

		virtual CoreObject *clone() const
		{
			return new FKObject( *this );
		}

	private:
		kroll::SharedValue m_data;
	};

	CoreObject* KObject_factory( const CoreClass* generator, void* data, bool )
	{
		kroll::SharedValue* vdata = (kroll::SharedValue*) data;
		return new FKObject( generator, *vdata );
	}

//===================================================
// Module declaration
//===================================================


//=====================================================
// Falcon side of the KMethod implementation
//
// This allows falcon script to access remote KMethod
// instances as wrapped in a falcon class.
//=====================================================

	class FKMethod: public CoreObject
	{
	public:
		FKMethod( const CoreClass* cls, kroll::SharedValue data ):
			CoreObject( cls ),
			m_data( data )
		{}

		FKMethod( const FKMethod& other ):
			CoreObject( other ),
			m_data( other.m_data )
		{}

		virtual ~FKMethod() {}

		virtual bool hasProperty( const String &key ) const
		{
			AutoCString ckey( key );
			return m_data->ToMethod()->HasProperty( ckey.c_str() );
		}

		virtual bool setProperty( const String &prop, const Item &value )
		{
			AutoCString ckey( prop );
			kroll::SharedValue v = kroll::FalconUtils::ToKrollValue( value );
			// In case of problems, this should fire an application specific exception.
			m_data->ToMethod()->Set( ckey.c_str(), v );

			return true;
		}

		virtual bool getProperty( const String &key, Item &ret ) const
		{
			// prepare a callable method for this method.
			if ( key == "call__" )
			{
				return defaultProperty( key, ret );
			}
			
			AutoCString ckey( key );
			// In case of problems, this should fire an application specific exception
			kroll::SharedValue v = m_data->ToMethod()->Get( ckey.c_str() );
			kroll::FalconUtils::ToFalconItem( v, ret );
			return true;
		}

		virtual CoreObject *clone() const
		{
			return new FKMethod( *this );
		}

		void call( VMachine* vm )
		{
			ArgList args;
			
			for( Falcon::uint32 pc = 0; vm->paramCount(); ++pc )
			{
				args.push_back( FalconUtils::ToKrollValue( *vm->param(pc) ) );
			}

			m_data->ToMethod()->Call( args );
		}
		
	private:
		kroll::SharedValue m_data;
	};

	CoreObject* KMethod_factory( const CoreClass* generator, void* data, bool )
	{
		kroll::SharedValue* vdata = (kroll::SharedValue*) data;
		return new FKMethod( generator, *vdata );
	}

	static void kmethod_call( Falcon::VMachine* vm )
	{
		// get the KRoll method
		FKMethod* self = Falcon::dyncast<FKMethod*>(vm->self().asObject());
		self->call( vm );
	}

//===================================================
// Module declaration
//===================================================

	Module* krollGlueModule()
	{
		Module *self = new Module();
		self->name( "kroll" );

		#define FALCON_DECLARE_MODULE self
		
		self->language( "en_US" );
		self->engineVersion( FALCON_VERSION_NUM );
		self->version( FALCON_KROLL_GLUE_MODULE_VERSION );

		// "%KObject"; the "%" marks it as CPP-private.
		Symbol* cls_kobject = self->addClass( FALCON_KOBJECT_CLASS_NAME ); 
		cls_kobject->getClassDef()->factory( &KObject_factory );
		cls_kobject->setWKS( true );
		// we don't add any standard property; all done via set/get/hasProperty.

		Symbol* cls_kmethod = self->addClass( FALCON_KMETHOD_CLASS_NAME );
		cls_kmethod->getClassDef()->factory( &KObject_factory );
		cls_kmethod->setWKS( true );
		// allow the KMethod object to be seen as a functor:
		self->addClassMethod( cls_kmethod, "call__", &kmethod_call );

		return self;
	}

}

