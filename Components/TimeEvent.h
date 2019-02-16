/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "SEGSEvent.h"

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
    uint64_t            m_timer_id;

                        Timeout(EventSrc *source=nullptr) : Event(evTimeout,source)
                            {
                            }
                        Timeout(const ACE_Time_Value &time, uint64_t dat,EventSrc *source)
                                : Event(evTimeout,source), m_arrival_time(time), m_timer_id(dat)
                        {
                        }
    uint64_t            timer_id() { return m_timer_id; }
    ACE_Time_Value      arrival_time() const { return m_arrival_time; }
    EVENT_IMPL(Timeout)
};
} // end of SEGSEvents namespace
