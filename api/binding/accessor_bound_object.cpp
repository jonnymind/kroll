/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <cstdio>
#include <cstring>

namespace kroll
{
	AccessorBoundObject::AccessorBoundObject(const char* name)
		: StaticBoundObject(name)
	{
	}

	SharedKMethod AccessorBoundObject::FindMethod(std::string& name)
	{
		std::string lcname = name;
		std::transform(lcname.begin(), lcname.end(), lcname.begin(), tolower);

		SharedStringList names = this->GetPropertyNames();
		for (size_t i = 0; i < names->size(); i++)
		{
			std::string other = *names->at(i);
			SharedValue v = this->RawGet(other.c_str());
			if (v->IsMethod()) {

				std::transform(other.begin(), other.end(), other.begin(), tolower);
				if (other == lcname) {
					return v->ToMethod();
				}

			}
		}
		return 0;
	}

	bool AccessorBoundObject::HasProperty(const char* name)
	{
		std::string styleOne = "get";
		std::string styleTwo = "is";
		styleOne.append(name);
		styleTwo.append(name);

		return StaticBoundObject::HasProperty(name) ||
			!this->FindMethod(styleOne).isNull() ||
			!this->FindMethod(styleTwo).isNull();
	}

	void AccessorBoundObject::Set(const char *name, SharedValue value)
	{
		std::string methodName = "set";
		methodName.append(name);
		SharedKMethod m = this->FindMethod(methodName);

		if (!m.isNull()) {
			m->Call(value);
		} else {
			this->RawSet(name, value);
		}
	}

	SharedValue AccessorBoundObject::Get(const char *name)
	{
		std::string styleOne = "get";
		std::string styleTwo = "is";
		styleOne.append(name);
		styleTwo.append(name);
		SharedKMethod m1 = this->FindMethod(styleOne);
		SharedKMethod m2 = this->FindMethod(styleTwo);

		if (!m1.isNull()) {
			return m1->Call();

		} else if (!m2.isNull()) {
			return m2->Call();

		} else {
			return this->RawGet(name);
		}
	}

	SharedValue AccessorBoundObject::RawGet(const char *name)
	{
		return StaticBoundObject::Get(name);
	}

	void AccessorBoundObject::RawSet(const char *name, SharedValue value)
	{
		StaticBoundObject::Set(name, value);
	}
}

