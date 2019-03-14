/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once
#include <functional>
/*!
 * @addtogroup Components
 * @{
 */
class EventSrc;
namespace SEGSEvents
{
class Event;
void register_event_type(const char *name, uint32_t type_id, std::function<Event *()> constructor);
Event *create_by_id(uint32_t type_id, EventSrc *src=nullptr);
Event *create_by_name(const char* name,EventSrc *src=nullptr);
const char *event_name(uint32_t type_id);
Event *from_storage(std::istream &istr);
void to_storage(std::ostream &ostr,Event *ev);
}
//! @}
