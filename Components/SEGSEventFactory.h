/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once
#include <functional>
/*!
 * @addtogroup Components
 * @{
 */
class EventProcessor;
namespace SEGSEvents
{
class Event;
void register_event_type(const char *name, uint32_t type_id, std::function<Event *()> constructor);
Event *create_by_id(uint32_t type_id,EventProcessor *src=nullptr);
Event *create_by_name(const char* name,EventProcessor *src=nullptr);
const char *event_name(uint32_t type_id);
}
//! @}
