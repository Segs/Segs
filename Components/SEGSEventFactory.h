/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once
#include <functional>
#include <streambuf>
/*!
 * @addtogroup Components
 * @{
 */
class EventSrc;
class QByteArray;

namespace SEGSEvents
{
class Event;
void register_event_type(const char *name, uint32_t type_id, std::function<Event *()> constructor);
Event *create_by_id(uint32_t type_id, EventSrc *src=nullptr);
Event *create_by_name(const char* name,EventSrc *src=nullptr);
const char *event_name(uint32_t type_id);

Event *from_storage(std::istream &istr);
void to_storage(std::ostream &ostr,Event *ev);

Event *from_byte_array(QByteArray &arr);
QByteArray to_byte_array(Event *ev);

}

class EventStreamBuffer : public std::streambuf
{
private :
	size_t m_size = 0;


	std::streamsize xsputn(const char* s, std::streamsize n) 
	{
		m_size += n;
		return std::streambuf::xsputn(s, n);
	}

public :
	size_t size()
	{
		return m_size;
	}
};

//! @}
