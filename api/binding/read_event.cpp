/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
namespace kroll
{
	ReadEvent::ReadEvent(AutoPtr<KEventObject> target, AutoBlob data) :
		Event(target, Event::READ),
		data(data)
	{
		this->SetMethod("getData", &ReadEvent::_GetData);
	}

	void ReadEvent::_GetData(const ValueList&, SharedValue result)
	{
		result->SetObject(this->data);
	}
}

