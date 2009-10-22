/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef __FALCON_CLASSES_H
#define __FALCON_CLASSES_H


namespace Falcon
{

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

		virtual ~FKObject();
		virtual bool hasProperty( const String &key ) const;
		virtual bool setProperty( const String &prop, const Item &value );
		virtual bool getProperty( const String &key, Item &ret ) const;
		virtual CoreObject *clone() const;

		kroll::SharedValue data() const { return m_data; }

	private:
		kroll::SharedValue m_data;
	};



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

		virtual ~FKMethod();
		virtual bool hasProperty( const String &key ) const;
		virtual bool setProperty( const String &prop, const Item &value );
		virtual bool getProperty( const String &key, Item &ret ) const;
		virtual CoreObject *clone() const;
		void call( VMachine* vm );

		kroll::SharedValue data() const { return m_data; }

	private:
		kroll::SharedValue m_data;
	};


//=====================================================
// Falcon side of the KList implementation
//
// This allows falcon script to access remote KList
// instances as wrapped in a falcon class.
//=====================================================

	class FKList: public CoreObject
	{
	public:
		FKList( const CoreClass* cls, kroll::SharedValue data ):
			CoreObject( cls ),
			m_data( data )
		{}

		FKList( const FKList& other ):
			CoreObject( other ),
			m_data( other.m_data )
		{}

		virtual ~FKList();

		virtual bool hasProperty( const String &key ) const;
		virtual bool setProperty( const String &prop, const Item &value );
		virtual bool getProperty( const String &key, Item &ret ) const;
		virtual CoreObject *clone() const;
		kroll::KList* klist() const { return m_data->ToList(); }

		kroll::SharedValue data() const { return m_data; }

		private:
			kroll::SharedValue m_data;
	};
}

#endif
