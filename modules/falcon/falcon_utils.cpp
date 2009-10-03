/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
 
#include "falcon_module.h"

namespace kroll
{
	void FalconUtils::ManageError( Falcon::Error* e )
	{
		Falcon::AutoCString desc( e->toString() );
		ValueException ve = ValueException::FromString( desc.c_str() ); // actually to UTF8
		e->decref();
		throw ve;
	}

	SharedValue FalconUtils::ToKrollValue(const Falcon::Item& item )
	{
		switch( item.type() )
		{
			case FLC_ITEM_NIL: return Value::Null;
			case FLC_ITEM_BOOL: return Value::NewBool( item.asBoolean() );
			case FLC_ITEM_INT: return Value::NewDouble( (double) item.asInteger() );
			case FLC_ITEM_NUM: return Value::NewDouble( (double) item.asNumeric() );
			case FLC_ITEM_STRING:
			{
				Falcon::AutoCString cstr( item.asString() );
				return Value::NewString( cstr.c_str() ); // actually, transfroms to UTF8
			}

			case FLC_ITEM_ARRAY: return Value::NewObject( new KFalconList( item.asArray() ) );
			case FLC_ITEM_OBJECT: return Value::NewObject( new KFalconObject( item.asObjectSafe() ) );
			case FLC_ITEM_METHOD: return Value::NewMethod( new KFalconMethod( item ) );
		}

		return Value::Undefined;
	}

	void FalconUtils::ToFalconItem( SharedValue value, Falcon::Item& item )
	{
		if (value->IsBool())
		{
			item = value->ToBool();
		}
		else if (value->IsNumber())
		{
			item = (Falcon::numeric) value->ToNumber();
		}
		else if (value->IsString())
		{
			Falcon::CoreString* cstr = new Falcon::CoreString;
			cstr->fromUTF8( value->ToString() );
			item = cstr;
		}
		else if (value->IsObject())
		{
			AutoPtr<KFalconObject> kfo = value.cast<KFalconObject>();
			if ( ! kfo.isNull() )
			{
				item = kfo->ToFalcon();
			}
			else
			{
				// TODO: use the pre-fetched instance saved int the Kroll FalconEvaluator instance.
				Falcon::VMachine* cvm = Falcon::VMachine::getCurrent();
				Falcon::Item* cls = cvm->findWKI( FALCON_KOBJECT_CLASS_NAME );
				item = cls->asClass()->createInstance( &value );
			}
		}
		else if (value->IsMethod())
		{
			AutoPtr<KFalconMethod> kfo = value.cast<KFalconMethod>();
			if ( ! kfo.isNull() )
			{
				item = kfo->ToFalcon();
			}
			else
			{
				// TODO: use the pre-fetched instance saved int the Kroll FalconEvaluator instance.
				Falcon::VMachine* cvm = Falcon::VMachine::getCurrent();
				Falcon::Item* cls = cvm->findWKI( FALCON_KMETHOD_CLASS_NAME );
				item = cls->asClass()->createInstance( &value );
			}
		}
		else if (value->IsList())
		{
			AutoPtr<KFalconList> kfl = value.cast<KFalconList>();
			if ( ! kfl.isNull() )
			{
				item = kfl->ToFalcon();
			}
			else
			{
				// TODO: use the pre-fetched instance saved int the Kroll FalconEvaluator instance.
				Falcon::VMachine* cvm = Falcon::VMachine::getCurrent();
				Falcon::Item* cls = cvm->findWKI( FALCON_KLIST_CLASS_NAME );
				item = cls->asClass()->createInstance( &value );
			}
		}
		else
			item.setNil();
	}

}
