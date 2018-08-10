/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <ace/Time_Value.h>
#include <atomic>
#include <typeinfo>
#include <cassert>
class EventProcessor;
namespace SEGSEvents
{
// Helper defines to ease the definition of event types
#define INIT_EVENTS() enum : uint32_t { base = 0,
#define BEGINE_EVENTS(parent_class) enum : uint32_t { base = parent_class::evLAST_EVENT,
#define BEGINE_EVENTS_INTERNAL() enum : uint32_t { base = 20000,
#define EVENT_DECL(name,cnt) name = base+cnt+1,
#define END_EVENTS(cnt) evLAST_EVENT=base+cnt+1};
class SEGS_EventTypes
{
public:
    INIT_EVENTS()
    EVENT_DECL(evFinish,0)  // this event will finish the Processor that receives it
    EVENT_DECL(evConnect,1) //! on the link level this means a new connection, higher level handlers are also notified by this event
    EVENT_DECL(evDisconnect,2)
    EVENT_DECL(evTimeout,3)
    END_EVENTS(4)
};

class Event
{
protected:
        const uint32_t  m_type;
        EventProcessor *m_event_source;
        std::atomic<int> m_ref_count {1}; // used to prevent event being deleted when it's in multiple queues

public:

virtual                 ~Event()
                        {
                            // we allow delete when there is 1 reference left (static variables on exit)
                            assert(m_ref_count<=1);
                            m_event_source=nullptr;
                        }
                        Event(uint32_t evtype,EventProcessor *ev_src=nullptr) :
                            m_type(evtype),m_event_source(ev_src)
                        {}
        Event *         shallow_copy() // just incrementing the ref count
                        {
                            ++m_ref_count;
                            return this;
                        }
        void            release()
                        {
                            if(0==--m_ref_count)
                                delete this;
                        }
        int             get_ref_count() const {return m_ref_count; }
        void            src(EventProcessor *ev_src) {m_event_source=ev_src;}
        EventProcessor *src() const {return m_event_source;}
        uint32_t        type() const {return m_type;}
virtual const char *    info();
};

// [[ev_def:type]]
class Timeout final: public Event
{
public:
                        // [[ev_def:field]]
    ACE_Time_Value      m_arrival_time;
                        // [[ev_def:field]]
    uint64_t            m_timer_id;


                        Timeout(const ACE_Time_Value &time, uint64_t dat,EventProcessor *source)
                                : Event(SEGS_EventTypes::evTimeout,source), m_arrival_time(time), m_timer_id(dat)
                        {
                        }
    uint64_t            timer_id() { return m_timer_id; }
    ACE_Time_Value      arrival_time() const { return m_arrival_time; }
};
// [[ev_def:type]]
struct Finish final: public Event
{
public:
                    Finish(EventProcessor *source=nullptr) : Event(SEGS_EventTypes::evFinish,source) {}
static  Finish *    s_instance;
};
} // end of SEGSEventsNamespace
