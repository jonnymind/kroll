/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "javascript_module.h"
#include <Poco/FileStream.h>

namespace kroll
{
namespace KJSUtil
{
	static inline Logger* GetLogger()
	{
		static Logger* logger = 0;
		if (!logger)
			logger = Logger::Get("JavaScript.KJSUtil");
		return logger;
	}

	static JSClassRef KJSKObjectClass = NULL;
	static JSClassRef KJSKMethodClass = NULL;
	static JSClassRef KJSKListClass = NULL;
	static const JSClassDefinition emptyClassDefinition =
	{
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0
	};

	// These are all KJSK*Class class methods.
	static void GetPropertyNamesCallback(JSContextRef, JSObjectRef, JSPropertyNameAccumulatorRef);
	static bool HasPropertyCallback(JSContextRef, JSObjectRef, JSStringRef);
	static JSValueRef GetPropertyCallback(JSContextRef, JSObjectRef, JSStringRef, JSValueRef*);
	static bool SetPropertyCallback(JSContextRef, JSObjectRef, JSStringRef, JSValueRef, JSValueRef*);
	static JSValueRef CallAsFunctionCallback(JSContextRef, JSObjectRef, JSObjectRef, size_t, const JSValueRef[], JSValueRef*);
	static void FinalizeCallback(JSObjectRef);
	static JSValueRef ToStringCallback(JSContextRef, JSObjectRef, JSObjectRef, size_t, const JSValueRef[], JSValueRef*);
	static JSValueRef EqualsCallback(JSContextRef, JSObjectRef, JSObjectRef, size_t, const JSValueRef[], JSValueRef*);

	// Private functions
	static void AddSpecialPropertyNames(SharedValue, SharedStringList, bool);
	static JSValueRef GetSpecialProperty(SharedValue, const char*, JSContextRef, SharedValue);
	static bool DoSpecialSetBehavior(SharedValue target, const char* name, SharedValue newValue);
	static JSValueRef GetFunctionPrototype(JSContextRef jsContext, JSValueRef* exception);
	static JSValueRef GetArrayPrototype(JSContextRef jsContext, JSValueRef* exception);

	SharedValue ToKrollValue(JSValueRef value, JSContextRef jsContext,
		JSObjectRef thisObject)
	{
		SharedValue krollValue = 0;
		JSValueRef exception = NULL;

		if (value == NULL)
		{
			GetLogger()->Error("Trying to convert NULL JSValueRef");
			return Value::Undefined;
		}

		if (JSValueIsNumber(jsContext, value))
		{
			krollValue = Value::NewDouble(JSValueToNumber(jsContext, value, &exception));
		}
		else if (JSValueIsBoolean(jsContext, value))
		{
			krollValue = Value::NewBool(JSValueToBoolean(jsContext, value));
		}
		else if (JSValueIsString(jsContext, value))
		{
			JSStringRef jsString = JSValueToStringCopy(jsContext, value, &exception);
			if (jsString)
			{
				std::string stringValue(ToChars(jsString));
				JSStringRelease(jsString);
				krollValue = Value::NewString(stringValue);
			}
		}
		else if (JSValueIsObject(jsContext, value))
		{
			JSObjectRef o = JSValueToObject(jsContext, value, &exception);
			if (o != NULL)
			{
				SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(o));
				if (value != NULL)
				{
					// This is a KJS-wrapped Kroll value: unwrap it
					return *value;
				}
				else if (JSObjectIsFunction(jsContext, o))
				{
					// this is a pure JS method: proxy it
					SharedKMethod tibm = new KKJSMethod(jsContext, o, thisObject);
					krollValue = Value::NewMethod(tibm);
				}
				else if (IsArrayLike(o, jsContext))
				{
					// this is a pure JS array: proxy it
					SharedKList tibl = new KKJSList(jsContext, o);
					krollValue = Value::NewList(tibl);
				}
				else
				{
					// this is a pure JS object: proxy it
					SharedKObject tibo = new KKJSObject(jsContext, o);
					krollValue = Value::NewObject(tibo);
				}
			}
		}
		else if (JSValueIsNull(jsContext, value))
		{
			krollValue = kroll::Value::Null;
		}
		else
		{
			krollValue = kroll::Value::Undefined;
		}
		if (!krollValue.isNull() && exception == NULL)
		{
			return krollValue;
		}
		else if (exception != NULL)
		{
			throw ToKrollValue(exception, jsContext, NULL);
		}
		else
		{
			GetLogger()->Error("Failed Kroll->JS conversion with no exception!");
			throw ValueException::FromString("Conversion from Kroll value to JS value failed");
		}
	}

	JSValueRef ToJSValue(SharedValue value, JSContextRef jsContext)
	{
		JSValueRef jsValue = NULL;
		if (value->IsInt())
		{
			jsValue = JSValueMakeNumber(jsContext, value->ToInt());
		}
		else if (value->IsDouble())
		{
			jsValue = JSValueMakeNumber(jsContext, value->ToDouble());
		}
		else if (value->IsBool())
		{
			jsValue = JSValueMakeBoolean(jsContext, value->ToBool());
		}
		else if (value->IsString())
		{
			JSStringRef s = JSStringCreateWithUTF8CString(value->ToString());
			jsValue = JSValueMakeString(jsContext, s);
			JSStringRelease(s);
		}
		else if (value->IsObject())
		{
			SharedKObject obj = value->ToObject();
			AutoPtr<KKJSObject> kobj = obj.cast<KKJSObject>();
			if (!kobj.isNull() && kobj->SameContextGroup(jsContext))
			{
				// this object is actually a pure JS object
				jsValue = kobj->GetJSObject();
			}
			else
			{
				// this is a KObject that needs to be proxied
				jsValue = KObjectToJSValue(value, jsContext);
			}
		}
		else if (value->IsMethod())
		{
			SharedKMethod meth = value->ToMethod();
			AutoPtr<KKJSMethod> kmeth = meth.cast<KKJSMethod>();
			if (!kmeth.isNull() && kmeth->SameContextGroup(jsContext))
			{
				// this object is actually a pure JS callable object
				jsValue = kmeth->GetJSObject();
			}
			else
			{
				// this is a KMethod that needs to be proxied
				jsValue = KMethodToJSValue(value, jsContext);
			}
		}
		else if (value->IsList())
		{
			SharedKList list = value->ToList();
			AutoPtr<KKJSList> klist = list.cast<KKJSList>();
			if (!klist.isNull() && klist->SameContextGroup(jsContext))
			{
				// this object is actually a pure JS array
				jsValue = klist->GetJSObject();
			}
			else
			{
				// this is a KList that needs to be proxied
				jsValue = KListToJSValue(value, jsContext);
			}
		}
		else if (value->IsNull())
		{
			jsValue = JSValueMakeNull(jsContext);
		}
		else if (value->IsUndefined())
		{
			jsValue = JSValueMakeUndefined(jsContext);
		}
		else
		{
			jsValue = JSValueMakeUndefined(jsContext);
		}

		return jsValue;

	}

	JSValueRef KObjectToJSValue(SharedValue objectValue, JSContextRef jsContext)
	{
		if (KJSKObjectClass == NULL)
		{
			JSClassDefinition jsClassDefinition = emptyClassDefinition;
			jsClassDefinition.className = "Object";
			jsClassDefinition.getPropertyNames = GetPropertyNamesCallback;
			jsClassDefinition.finalize = FinalizeCallback;
			jsClassDefinition.hasProperty = HasPropertyCallback;
			jsClassDefinition.getProperty = GetPropertyCallback;
			jsClassDefinition.setProperty = SetPropertyCallback;
			KJSKObjectClass = JSClassCreate(&jsClassDefinition);
		}
		return JSObjectMake(jsContext, KJSKObjectClass, new SharedValue(objectValue));
	}

	JSValueRef KMethodToJSValue(SharedValue methodValue, JSContextRef jsContext)
	{
		if (KJSKMethodClass == NULL)
		{
			JSClassDefinition jsClassDefinition = emptyClassDefinition;
			jsClassDefinition.className = "Function";
			jsClassDefinition.getPropertyNames = GetPropertyNamesCallback;
			jsClassDefinition.finalize = FinalizeCallback;
			jsClassDefinition.hasProperty = HasPropertyCallback;
			jsClassDefinition.getProperty = GetPropertyCallback;
			jsClassDefinition.setProperty = SetPropertyCallback;
			jsClassDefinition.callAsFunction = CallAsFunctionCallback;
			KJSKMethodClass = JSClassCreate(&jsClassDefinition);
		}
		JSObjectRef jsobject = JSObjectMake(jsContext, KJSKMethodClass, new SharedValue(methodValue));
		JSValueRef functionPrototype = GetFunctionPrototype(jsContext, NULL);
		JSObjectSetPrototype(jsContext, jsobject, functionPrototype);
		return jsobject;
	}

	JSValueRef KListToJSValue(SharedValue listValue, JSContextRef jsContext)
	{

		if (KJSKListClass == NULL)
		{
			JSClassDefinition jsClassDefinition = emptyClassDefinition;
			jsClassDefinition.className = "Array";
			jsClassDefinition.getPropertyNames = GetPropertyNamesCallback;
			jsClassDefinition.finalize = FinalizeCallback;
			jsClassDefinition.hasProperty = HasPropertyCallback;
			jsClassDefinition.getProperty = GetPropertyCallback;
			jsClassDefinition.setProperty = SetPropertyCallback;
			KJSKListClass = JSClassCreate(&jsClassDefinition);
		}

		JSObjectRef jsobject = JSObjectMake(jsContext, KJSKListClass, new SharedValue(listValue));
		JSValueRef arrayPrototype = GetArrayPrototype(jsContext, NULL);
		JSObjectSetPrototype(jsContext, jsobject, arrayPrototype);
		return jsobject;
	}

	std::string ToChars(JSStringRef jsString)
	{
		size_t size = JSStringGetMaximumUTF8CStringSize(jsString);
		char* cstring = (char*) malloc(size);
		JSStringGetUTF8CString(jsString, cstring, size);
		std::string string(cstring);
		free(cstring);
		return string;
	}

	bool IsArrayLike(JSObjectRef object, JSContextRef jsContext)
	{
		bool isArrayLike = true;

		JSStringRef pop = JSStringCreateWithUTF8CString("pop");
		isArrayLike = isArrayLike && JSObjectHasProperty(jsContext, object, pop);
		JSStringRelease(pop);

		JSStringRef concat = JSStringCreateWithUTF8CString("concat");
		isArrayLike = isArrayLike && JSObjectHasProperty(jsContext, object, concat);
		JSStringRelease(concat);

		JSStringRef length = JSStringCreateWithUTF8CString("length");
		isArrayLike = isArrayLike && JSObjectHasProperty(jsContext, object, length);
		JSStringRelease(length);

		return isArrayLike;
	}

	static void FinalizeCallback(JSObjectRef jsObject)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(jsObject));
		delete value;
	}

	static bool PrototypeHasFunctionNamed(JSContextRef jsContext, JSObjectRef object,
		JSStringRef name)
	{
		JSValueRef exception = NULL;

		JSValueRef prototypeValue = JSObjectGetPrototype(jsContext, object);
		JSObjectRef prototype = JSValueToObject(jsContext, prototypeValue, &exception);

		if (exception)
			return false;

		JSValueRef propValue = JSObjectGetProperty(jsContext, prototype, name, &exception);
		if (exception)
			return false;

		if (!JSValueIsObject(jsContext, propValue))
			return false;

		JSObjectRef prop = JSValueToObject(jsContext, propValue, &exception);
		return !exception && JSObjectIsFunction(jsContext, prop);
	}

	static bool HasPropertyCallback(JSContextRef jsContext, JSObjectRef jsObject,
		JSStringRef jsProperty)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(jsObject));
		if (value == NULL)
			return false;

		// Convert the name to a std::string.
		SharedKObject object = (*value)->ToObject();
		std::string name(ToChars(jsProperty));

		// Special properties always take precedence. This is important
		// because even though the Array and Function prototypes have 
		// methods like toString -- we always want our special properties
		// to override those.
		SharedStringList specialProperties(new StringList());
		AddSpecialPropertyNames(*value, specialProperties, true);
		for (size_t i = 0; i < specialProperties->size(); i++)
		{
			if (name == *specialProperties->at(i))
				return true;
		}

		// If the JavaScript prototype for Lists (Array) or Methods (Function) has
		// a method with the same name -- opt to use the prototype's version instead.
		// This will prevent  incompatible versions of things like pop() bleeding into
		// JavaScript.
		if (((*value)->IsList() || (*value)->IsMethod()) &&
			PrototypeHasFunctionNamed(jsContext, jsObject, jsProperty))
		{
			return false;
		}

		return object->HasProperty(name.c_str());
	}

	static JSValueRef GetPropertyCallback(JSContextRef jsContext, 
		JSObjectRef jsObject, JSStringRef jsProperty, JSValueRef* jsException)
	{

		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(jsObject));
		if (value == NULL)
			return JSValueMakeUndefined(jsContext);

		SharedKObject object = (*value)->ToObject();
		std::string name(ToChars(jsProperty));
		JSValueRef jsValue = NULL;
		try
		{
			SharedValue kvalue = object->Get(name.c_str());
			jsValue = GetSpecialProperty(*value, name.c_str(), jsContext, kvalue);
		}
		catch (ValueException& exception)
		{
			*jsException = ToJSValue(exception.GetValue(), jsContext);
		}
		catch (std::exception &e)
		{
			SharedValue v = Value::NewString(e.what());
			*jsException = ToJSValue(v, jsContext);
		}
		catch (...)
		{
			std::string error = "Unknown exception trying to get property: ";
			error.append(name);
			SharedValue v = Value::NewString(error);
			*jsException = ToJSValue(v, jsContext);
		}

		return jsValue;
	}

	static bool SetPropertyCallback(JSContextRef jsContext, JSObjectRef jsObject,
		JSStringRef jsProperty, JSValueRef jsValue, JSValueRef* jsException)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(jsObject));
		if (value == NULL)
			return false;

		SharedKObject object = (*value)->ToObject();
		bool success = false;
		std::string propertyName(ToChars(jsProperty));
		try
		{
			SharedValue newValue = ToKrollValue(jsValue, jsContext, jsObject);

			// Arrays in particular have a special behavior when
			// you do something like set the "length" property
			if (!DoSpecialSetBehavior(*value, propertyName.c_str(), newValue))
			{
				object->Set(propertyName.c_str(), newValue);
			}
			success = true;
		}
		catch (ValueException& exception)
		{
			*jsException = ToJSValue(exception.GetValue(), jsContext);
		}
		catch (std::exception &e)
		{
			SharedValue v = Value::NewString(e.what());
			*jsException = ToJSValue(v, jsContext);
		}
		catch (...)
		{
			std::string error = "Unknown exception trying to set property: ";
			error.append(propertyName);
			SharedValue v = Value::NewString(error);
			*jsException = ToJSValue(v, jsContext);
		}

		return success;
	}

	static JSValueRef CallAsFunctionCallback(JSContextRef jsContext,
		JSObjectRef jsFunction, JSObjectRef jsThis, size_t argCount,
		const JSValueRef jsArgs[], JSValueRef* jsException)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(jsFunction));
		if (value == NULL)
			return JSValueMakeUndefined(jsContext);

		SharedKMethod method = (*value)->ToMethod();
		ValueList args;
		for (size_t i = 0; i < argCount; i++)
		{
			SharedValue argVal = ToKrollValue(jsArgs[i], jsContext, jsThis);
			Value::Unwrap(argVal);
			args.push_back(argVal);
		}

		JSValueRef jsValue = NULL;
		try
		{
			SharedValue kvalue = method->Call(args);
			jsValue = ToJSValue(kvalue, jsContext);
		}
		catch (ValueException& exception)
		 {
			SharedString str = exception.DisplayString();
			*jsException = ToJSValue(exception.GetValue(), jsContext);
		} 
		catch (std::exception &e)
		{
			SharedValue v = Value::NewString(e.what());
			*jsException = ToJSValue(v, jsContext);
		}
		catch (...)
		{
			SharedValue v = Value::NewString("Unknown exception during Kroll method call");
			*jsException = ToJSValue(v, jsContext);
		}

		return jsValue;
	}

	static void AddSpecialPropertyNames(SharedValue value, SharedStringList props,
		bool showInvisible)
	{
		// Some attributes should be hidden unless the are requested specifically -- 
		// essentially a has_property(...) versus  get_property_list(...). An example
		// of this type of attribute is toString(). Some JavaScript code might expect
		// a "hash" object to have no methods in its property list. We don't want
		// toString() to show up in those situations.

		bool foundLength = false, foundToString = false, foundEquals = false;
		for (size_t i = 0; i < props->size(); i++)
		{
			SharedString propertyName = props->at(i);
			if (strcmp(propertyName->c_str(), "length") == 0)
				foundLength = true;
			if (strcmp(propertyName->c_str(), "toString") == 0)
				foundToString = true;
			if (strcmp(propertyName->c_str(), "equals") == 0)
				foundEquals = true;
		}

		if (!foundLength && value->IsList())
		{
			props->push_back(new std::string("length"));
		}

		if (!foundToString && showInvisible)
		{
			props->push_back(new std::string("toString"));
		}

		if (!foundEquals && showInvisible)
		{
			props->push_back(new std::string("equals"));
		}
	}

	static JSValueRef GetSpecialProperty(SharedValue value, const char* name, 
		JSContextRef jsContext, SharedValue objValue)
	{
		// Always override the length property on lists. Some languages
		// supply their own length property, which might be a method instead
		// of a number -- bad news.
		if (value->IsList() && !strcmp(name, "length"))
		{
			SharedKList l = value->ToList();
			return JSValueMakeNumber(jsContext, l->Size());
		}

		// Only overload these methods if the value in our object is not a
		// method We want the user to be able to supply their own versions,
		// but we don't want JavaScript code to freak out in situations where
		// Kroll objects use attributes with the same name that aren't methods.
		if (!objValue->IsMethod())
		{
			if (!strcmp(name, "toString"))
			{
				JSStringRef s = JSStringCreateWithUTF8CString("toString");
				return JSObjectMakeFunctionWithCallback(jsContext, s, &ToStringCallback);
			}

			if (!strcmp(name, "equals"))
			{
				JSStringRef s = JSStringCreateWithUTF8CString("equals");
				return JSObjectMakeFunctionWithCallback(jsContext, s, &EqualsCallback);
			}
		}

		// Otherwise this is just a normal JS value
		return ToJSValue(objValue, jsContext);
	}

	static bool DoSpecialSetBehavior(SharedValue target, const char* name, SharedValue newValue)
	{
		// We only do something special if we are trying to set
		// the length property of a list to a new int value.
		if (strcmp(name, "length") || !target->IsList() || !newValue->IsInt())
		{
			return false;
		}
		target->ToList()->ResizeTo(newValue->ToInt());
		return true;
	}

	static JSValueRef ToStringCallback(JSContextRef jsContext,
		JSObjectRef jsFunction, JSObjectRef jsThis, size_t argCount,
		const JSValueRef args[], JSValueRef* exception)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(jsThis));
		if (value == NULL)
			return JSValueMakeUndefined(jsContext);

		SharedString ss = (*value)->DisplayString(2);
		SharedValue dsv = Value::NewString(ss);
		return ToJSValue(dsv, jsContext);
	}

	static JSValueRef EqualsCallback(JSContextRef jsContext, JSObjectRef function,
		JSObjectRef jsThis, size_t numArgs, const JSValueRef args[],
		JSValueRef* exception)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(jsThis));
		if (value == NULL || numArgs < 1)
		{
			return JSValueMakeBoolean(jsContext, false);
		}

		// Ensure argument is a JavaScript object
		if (!JSValueIsObject(jsContext, args[0]))
		{
			return JSValueMakeBoolean(jsContext, false);
		}

		// Ensure argument is a Kroll JavaScript
		JSObjectRef otherObject = JSValueToObject(jsContext, args[0], NULL);
		SharedValue* otherValue = static_cast<SharedValue*>(JSObjectGetPrivate(otherObject));
		if (otherValue == NULL)
		{
			return JSValueMakeBoolean(jsContext, false);
		}

		// Test equality
		return JSValueMakeBoolean(jsContext, (*value)->Equals(*otherValue));
	}

	static void GetPropertyNamesCallback(JSContextRef jsContext,
		JSObjectRef jsObject, JSPropertyNameAccumulatorRef jsProperties)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(jsObject));
		if (value == NULL)
			return;

		SharedKObject object = (*value)->ToObject();
		SharedStringList props = object->GetPropertyNames();
		AddSpecialPropertyNames(*value, props, false);
		for (size_t i = 0; i < props->size(); i++)
		{
			SharedString propertyName = props->at(i);
			JSStringRef name = JSStringCreateWithUTF8CString(propertyName->c_str());
			JSPropertyNameAccumulatorAddName(jsProperties, name);
			JSStringRelease(name);
		}
	}

	JSObjectRef CreateNewGlobalContext(Host *host, bool addGlobalObject)
	{
		JSGlobalContextRef jsContext = JSGlobalContextCreate(NULL);
		JSObjectRef globalObject = JSContextGetGlobalObject(jsContext);
		RegisterGlobalContext(globalObject, jsContext);
		
		if (addGlobalObject)
		{
			/* Take some steps to insert the API into the Javascript jsContext */
			/* Create a crazy, crunktown delegate hybrid object for Javascript */
			SharedValue globalValue = Value::NewObject(host->GetGlobalObject());

			/* convert JS API to a KJS object */
			JSValueRef jsAPI = ToJSValue(globalValue, jsContext);

			/* set the API as a property of the global object */
			JSStringRef propertyName = JSStringCreateWithUTF8CString(PRODUCT_NAME);
			JSObjectSetProperty(jsContext, globalObject, propertyName,
				jsAPI, kJSPropertyAttributeNone, NULL);

		}
		
		return globalObject;
	}

	static std::map<JSObjectRef, JSGlobalContextRef> jsContextMap;
	void RegisterGlobalContext(
		JSObjectRef object,
		JSGlobalContextRef globalContext)
	{
		jsContextMap[object] = globalContext;
	}
	
	void UnregisterGlobalContext(JSObjectRef object)
	{
		std::map<JSObjectRef, JSGlobalContextRef>::iterator i = jsContextMap.find(object);
		if (i!=jsContextMap.end())
		{
			jsContextMap.erase(i);
		}
	}
	

	JSGlobalContextRef GetGlobalContext(JSObjectRef object)
	{
		if (jsContextMap.find(object) == jsContextMap.end())
		{
			return NULL;
		}
		else
		{
			return jsContextMap[object];
		}
	}

	static std::map<JSGlobalContextRef, int> jsContextRefCounts;
	void ProtectGlobalContext(JSGlobalContextRef globalContext)
	{
		if (jsContextRefCounts.find(globalContext) == jsContextRefCounts.end())
		{
			JSGlobalContextRetain(globalContext);
			jsContextRefCounts[globalContext] = 1;
		}
		else
		{
			jsContextRefCounts[globalContext]++;
		}
	}

	void UnprotectGlobalContext(JSGlobalContextRef globalContext)
	{
		std::map<JSGlobalContextRef, int>::iterator i
			= jsContextRefCounts.find(globalContext);

		if (i == jsContextRefCounts.end())
		{
			GetLogger()->Error("Tried to unprotect an unknown jsContext!");
		}
		else if (i->second == 1)
		{
			JSGlobalContextRelease(globalContext);
			jsContextRefCounts.erase(i);
		}
		else
		{
			jsContextRefCounts[globalContext]--;
		}
	}

	SharedValue Evaluate(JSContextRef jsContext, char *script)
	{
		JSObjectRef globalObject = JSContextGetGlobalObject(jsContext);
		JSStringRef scriptContents = JSStringCreateWithUTF8CString(script);
		JSStringRef url = JSStringCreateWithUTF8CString("<string>");
		JSValueRef exception = NULL;

		JSValueRef returnValue = JSEvaluateScript(jsContext, scriptContents, globalObject, 
			url, 0, &exception);

		JSStringRelease(url);
		JSStringRelease(scriptContents);

		if (exception != NULL)
		{
			throw ValueException(ToKrollValue(exception, jsContext, NULL));
		}

		return ToKrollValue(returnValue, jsContext, globalObject);
	}

	SharedValue EvaluateFile(JSContextRef jsContext, std::string fullPath)
	{
		GetLogger()->Debug("Evaluating JavaScript file at: %s", fullPath.c_str());

		JSObjectRef globalObject = JSContextGetGlobalObject(jsContext);
		std::string scriptContents(FileUtils::ReadFile(fullPath));

		JSStringRef script = JSStringCreateWithUTF8CString(scriptContents.c_str());
		JSStringRef jsFullPath = JSStringCreateWithUTF8CString(fullPath.c_str());
		JSValueRef exception = NULL;
		JSValueRef returnValue = JSEvaluateScript(jsContext, script, globalObject, 
			jsFullPath, 0, &exception);
		JSStringRelease(script);
		JSStringRelease(jsFullPath);

		if (exception != NULL)
		{
			throw ValueException(ToKrollValue(exception, jsContext, NULL));
		}

		return ToKrollValue(returnValue, jsContext, globalObject);
	}

	//===========================================================================//
	// METHODS BORROWED ARE TAKEN FROM GWT - modifications under same license
	//===========================================================================//
	/*
	 * Copyright 2008 Google Inc.
	 * 
	 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
	 * use this file except in compliance with the License. You may obtain a copy of
	 * the License at
	 * 
	 * http://www.apache.org/licenses/LICENSE-2.0
	 * 
	 * Unless required by applicable law or agreed to in writing, software
	 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
	 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
	 * License for the specific language governing permissions and limitations under
	 * the License.
	 */
	
	/*
	 * The following takes the prototype from the Function constructor, this allows
	 * us to easily support call and apply on our objects that support CallAsFunction.
	 *
	 * NOTE: The return value is not protected.
	 */
	static JSValueRef GetFunctionPrototype(JSContextRef jsContext, JSValueRef* exception) 
	{
		JSObjectRef globalObject = JSContextGetGlobalObject(jsContext);
		JSStringRef fnPropName = JSStringCreateWithUTF8CString("Function");
		JSValueRef fnCtorValue = JSObjectGetProperty(jsContext, globalObject,
			fnPropName, exception);
		JSStringRelease(fnPropName);
		if (!fnCtorValue)
		{
			return JSValueMakeUndefined(jsContext);
		}

		JSObjectRef fnCtorObject = JSValueToObject(jsContext, fnCtorValue, exception);
		if (!fnCtorObject)
		{
			return JSValueMakeUndefined(jsContext);
		}

		JSStringRef protoPropName = JSStringCreateWithUTF8CString("prototype");
		JSValueRef fnPrototype = JSObjectGetProperty(jsContext, fnCtorObject,
			protoPropName, exception);
		JSStringRelease(protoPropName);
		if (!fnPrototype)
		{
			return JSValueMakeUndefined(jsContext);
		}

	return fnPrototype;
	}

	/*
	 * The following takes the prototype from the Array constructor, this allows
	 * us to easily support array-like functions
	 *
	 * NOTE: The return value is not protected.
	 */
	static JSValueRef GetArrayPrototype(JSContextRef jsContext, JSValueRef* exception) 
	{
		JSObjectRef globalObject = JSContextGetGlobalObject(jsContext);
		JSStringRef fnPropName = JSStringCreateWithUTF8CString("Array");
		JSValueRef fnCtorValue = JSObjectGetProperty(jsContext, globalObject,
			fnPropName, exception);
		JSStringRelease(fnPropName);
		if (!fnCtorValue) 
		{
			return JSValueMakeUndefined(jsContext);
		}

		JSObjectRef fnCtorObject = JSValueToObject(jsContext, fnCtorValue, exception);
		if (!fnCtorObject)
		{
			return JSValueMakeUndefined(jsContext);
		}

		JSStringRef protoPropName = JSStringCreateWithUTF8CString("prototype");
		JSValueRef fnPrototype = JSObjectGetProperty(jsContext, fnCtorObject,
			protoPropName, exception);
		JSStringRelease(protoPropName);
		if (!fnPrototype) 
		{
			return JSValueMakeUndefined(jsContext);
		}

		return fnPrototype;
	}
	
	void BindProperties(JSObjectRef globalObject, SharedKObject obj)
	{
		JSGlobalContextRef jsContext = GetGlobalContext(globalObject);

		SharedStringList names = obj->GetPropertyNames();
		for (size_t i = 0; i < names->size(); i++)
		{
			std::string other = *names->at(i);
			SharedValue v = obj->Get(other.c_str());
			JSValueRef js = ToJSValue(v, jsContext);
			JSStringRef propertyName = JSStringCreateWithUTF8CString(other.c_str());
			JSObjectSetProperty(jsContext, globalObject, propertyName, js, kJSPropertyAttributeNone, NULL);
		}
	}

	SharedValue GetProperty(JSObjectRef globalObject, std::string name)
	{
		JSGlobalContextRef jsContext = GetGlobalContext(globalObject);
		JSStringRef jsName = JSStringCreateWithUTF8CString(name.c_str());
		JSValueRef prop = JSObjectGetProperty(jsContext, globalObject, jsName, NULL);
		JSStringRelease(jsName);
		return ToKrollValue(prop, jsContext, globalObject);
	}
}
}
