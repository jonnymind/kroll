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

	FKObject::~FKObject() {}

	bool FKObject::hasProperty( const String &key ) const
	{
		AutoCString ckey( key );
		return m_data->ToObject()->HasProperty( ckey.c_str() );
	}

	bool FKObject::setProperty( const String &prop, const Item &value )
	{
		AutoCString ckey( prop );
		kroll::SharedValue v = kroll::FalconUtils::ToKrollValue( value );
		// In case of problems, this should fire an application specific exception.
		m_data->ToObject()->Set( ckey.c_str(), v );

		return true;
	}

	bool FKObject::getProperty( const String &key, Item &ret ) const
	{
		AutoCString ckey( key );
		if ( m_data->ToObject()->HasProperty( ckey.c_str() ) )
		{
			// In case of problems, this should fire an application specific exception
			kroll::SharedValue v = m_data->ToObject()->Get( ckey.c_str() );
			kroll::FalconUtils::ToFalconItem( v, ret );
			return true;
		}
		// return one of our properties... or raise an error.
		return defaultProperty( key, ret );
	}

	CoreObject *FKObject::clone() const
	{
		return new FKObject( *this );
	}


	CoreObject* KObject_factory( const CoreClass* generator, void* data, bool )
	{
		kroll::SharedValue* vdata = (kroll::SharedValue*) data;
		return new FKObject( generator, *vdata );
	}

//=====================================================
// Falcon side of the KMethod implementation
//
// This allows falcon script to access remote KMethod
// instances as wrapped in a falcon class.
//=====================================================


	FKMethod::~FKMethod() {}

	bool FKMethod::hasProperty( const String &key ) const
	{
		AutoCString ckey( key );
		return m_data->ToMethod()->HasProperty( ckey.c_str() );
	}

	bool FKMethod::setProperty( const String &prop, const Item &value )
	{
		AutoCString ckey( prop );
		kroll::SharedValue v = kroll::FalconUtils::ToKrollValue( value );
		// In case of problems, this should fire an application specific exception.
		m_data->ToMethod()->Set( ckey.c_str(), v );

		return true;
	}

	bool FKMethod::getProperty( const String &key, Item &ret ) const
	{
		Logger::Get("Falcon")->Debug( "getting property" );
		// prepare a callable method for this method.
		if ( key == "call__" )
		{
			Logger::Get("Falcon")->Debug( "getting call__ property" );
			return defaultProperty( key, ret );
		}

		AutoCString ckey( key );
		// In case of problems, this should fire an application specific exception
		kroll::SharedValue v = m_data->ToMethod()->Get( ckey.c_str() );
		kroll::FalconUtils::ToFalconItem( v, ret );
		return true;
	}

	CoreObject *FKMethod::clone() const
	{
		return new FKMethod( *this );
	}

	void FKMethod::call( VMachine* vm )
	{
		ArgList args;
		Logger::Get("Falcon")->Debug( "Calling method with %d values", vm->paramCount() );

		for( int pc = 0; pc < vm->paramCount(); ++pc )
		{
			args.push_back( FalconUtils::ToKrollValue( *vm->param(pc) ) );
		}

		FalconUtils::ToFalconItem( m_data->ToMethod()->Call( args ), vm->regA() );
	}


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

//=====================================================
// Falcon side of the KList implementation
//
// This allows falcon script to access remote KList
// instances as wrapped in a falcon class.
//=====================================================

	
	FKList::~FKList() {}

	bool FKList::hasProperty( const String &key ) const
	{
		uint32 pos = 0;
		return generator()->properties().findKey(key, pos);
	}

	bool FKList::setProperty( const String &prop, const Item &value )
	{
		return false;
	}

	bool FKList::getProperty( const String &key, Item &ret ) const
	{
		return defaultProperty( key, ret );
	}

	CoreObject *FKList::clone() const
	{
		return new FKList( *this );
	}


	CoreObject* FKList_factory( const CoreClass* generator, void* data, bool )
	{
		kroll::SharedValue* vdata = (kroll::SharedValue*) data;
		return new FKList( generator, *vdata );
	}

	static void FKList_At( Falcon::VMachine* vm )
	{
		// check parameters
		Falcon::Item* i_index = vm->param( 0 );
		
		if( i_index == 0 || ! i_index->isOrdinal() )
		{
			throw new Falcon::ParamError( Falcon::ErrorParam( Falcon::e_inv_params, __LINE__ )
				.extra( "N" ) );
		}
		
		// get the KRoll method
		FKList* self = Falcon::dyncast<FKList*>(vm->self().asObject());
		KList* list = self->klist();

		// get the item and return it in A
		FalconUtils::ToFalconItem( list->At( (unsigned int) i_index->forceInteger() ), vm->regA() );
	}

	static void FKList_SetAt( Falcon::VMachine* vm )
	{
		// check parameters
		Falcon::Item* i_index = vm->param( 0 );
		Falcon::Item* i_value = vm->param( 1 );

		if( i_index == 0 || ! i_index->isOrdinal() || i_value == 0 )
		{
			throw new Falcon::ParamError( Falcon::ErrorParam( Falcon::e_inv_params, __LINE__ )
				.extra( "N,X" ) );
		}

		// get the KRoll method
		FKList* self = Falcon::dyncast<FKList*>(vm->self().asObject());
		KList* list = self->klist();

		Falcon::int64 index = i_index->forceInteger();
		if( index < 0 || index >= (Falcon::int64) list->Size() )
		{
			throw new Falcon::ParamError( Falcon::ErrorParam( Falcon::e_param_range, __LINE__ ) );
		}
		
		// get the item and return it in A
		list->SetAt( (unsigned int) index, FalconUtils::ToKrollValue( *i_value ) );
	}

	static void FKList_Size( Falcon::VMachine* vm )
	{
		// get the KRoll method
		FKList* self = Falcon::dyncast<FKList*>(vm->self().asObject());
		KList* list = self->klist();
		vm->retval( (Falcon::int64) list->Size() );
	}

	static void FKList_Append( Falcon::VMachine* vm )
	{
		// check parameters
		Falcon::Item* i_value = vm->param( 0 );

		if( i_value == 0 )
		{
			throw new Falcon::ParamError( Falcon::ErrorParam( Falcon::e_inv_params, __LINE__ )
				.extra( "X" ) );
		}

		// get the KRoll method
		FKList* self = Falcon::dyncast<FKList*>(vm->self().asObject());
		KList* list = self->klist();
		list->Append( FalconUtils::ToKrollValue( *i_value ) );
	}

	static void FKList_Remove( Falcon::VMachine* vm )
	{
		// check parameters
		Falcon::Item* i_index = vm->param( 0 );

		if( i_index == 0 || ! i_index->isOrdinal() )
		{
			throw new Falcon::ParamError( Falcon::ErrorParam( Falcon::e_inv_params, __LINE__ )
				.extra( "N" ) );
		}

		// get the KRoll method
		FKList* self = Falcon::dyncast<FKList*>(vm->self().asObject());
		KList* list = self->klist();

		Falcon::int64 index = i_index->forceInteger();
		if( index < 0 || index >= (Falcon::int64) list->Size() )
		{
			throw new Falcon::ParamError( Falcon::ErrorParam( Falcon::e_param_range, __LINE__ ) );
		}
		
		vm->regA().setBoolean( list->Remove( (unsigned int) index ) );
	}

//===================================================
// Useful functions for falcon SCRIPTS
//===================================================

static void KFObkect_GetPropertyNames( Falcon::VMachine* vm )
{
	FKObject* fko = dyncast<FKObject*>( vm->self().asObject() );
	SharedStringList ssl = fko->data()->ToObject()->GetPropertyNames();

	CoreArray* ca = new CoreArray;
	for( uint32 i = 0; i < ssl->size(); ++i )
	{
		CoreString* cs = new CoreString;
		cs->fromUTF8( ssl->at(i)->c_str() );
		ca->append( cs );
	}
	vm->retval( ca );
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

		// Add the window global object.
		self->addGlobal( "window" )->setWKS( true );
		self->addGlobal( "document" )->setWKS( true );
		
		// "%KObject"; the "%" marks it as CPP-private.
		Symbol* cls_kobject = self->addClass( FALCON_KOBJECT_CLASS_NAME ); 
		cls_kobject->getClassDef()->factory( &KObject_factory );
		cls_kobject->setWKS( true );
		self->addClassMethod( cls_kobject, "GetPropertyNames", &KFObkect_GetPropertyNames );
		
		// we don't add any standard property; all done via set/get/hasProperty.

		Symbol* cls_kmethod = self->addClass( FALCON_KMETHOD_CLASS_NAME );
		cls_kmethod->getClassDef()->factory( &KMethod_factory );
		cls_kmethod->setWKS( true );
		// allow the KMethod object to be seen as a functor:
		self->addClassMethod( cls_kmethod, "call__", &kmethod_call );

		// reflect the KList
		Symbol* cls_klist = self->addClass( FALCON_KLIST_CLASS_NAME );
		cls_klist->getClassDef()->factory( &FKList_factory );
		cls_klist->setWKS( true );

		// allow the KMethod object to be seen as a functor:
		self->addClassMethod( cls_klist, "At", &FKList_At ).asSymbol()
			->addParam( "index" );
		self->addClassMethod( cls_klist, "SetAt", &FKList_SetAt ).asSymbol()
			->addParam( "index" )->addParam( "value" );
		self->addClassMethod( cls_klist, "Size", &FKList_Size );
		self->addClassMethod( cls_klist, "Append", &FKList_Append ).asSymbol()
			->addParam( "value" );
		self->addClassMethod( cls_klist, "Remove", &FKList_Remove ).asSymbol()
			->addParam( "index" );
		
		return self;
	}

}

