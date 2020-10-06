/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/SEGSEvent.h"

#include <ace/Time_Value.h>
namespace SEGSEvents
{
// [[ev_def:type]]
class Timeout final: public Event
{
public:
                        // [[ev_def:field]]
    ACE_Time_Value      m_arrival_time;
                        // [[ev_def:field]]
    uint32_t            m_timer_id;

                        Timeout(EventSrc *source=nullptr) : Event(evTimeout,source)
                        {
                        }
                        Timeout(const ACE_Time_Value &time, uint32_t dat,EventSrc *source)
                                : Event(evTimeout,source), m_arrival_time(time), m_timer_id(dat)
                        {
                        }
    uint32_t            timer_id() const { return m_timer_id; }
    ACE_Time_Value      arrival_time() const { return m_arrival_time; }

    EVENT_IMPL(Timeout)
};
} // end of SEGSEvents namespace
