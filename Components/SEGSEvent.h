/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <ace/Time_Value.h>
#include "cereal/archives/binary.hpp"
#include <atomic>
#include <typeinfo>
#include <cassert>

namespace CompileTimeUtils
{
constexpr uint32_t val_32_const     = 0x811c9dc5;
constexpr uint32_t prime_32_const   = 0x1000193;
constexpr uint64_t val_64_const     = 0xcbf29ce484222325;
constexpr uint64_t prime_64_const   = 0x100000001b3;

inline constexpr uint32_t hash_32_fnv1a_const(const char* const str, const uint32_t value = val_32_const) noexcept {
    return (str[0] == '\0') ? value : hash_32_fnv1a_const(&str[1], (value ^ uint32_t(str[0])) * prime_32_const);
}

inline constexpr uint64_t hash_64_fnv1a_const(const char* const str, const uint64_t value = val_64_const) noexcept {
    return (str[0] == '\0') ? value : hash_64_fnv1a_const(&str[1], (value ^ uint64_t(str[0])) * prime_64_const);
}
}

class EventSrc;
namespace SEGSEvents
{
// Helper macros to ease the definition of event types
#define BEGINE_EVENTS(enum_name,parent_class) base_##enum_name = (uint32_t)parent_class::ID_LAST_##parent_class,
#define BEGINE_EVENTS_SKIP(enum_name,parent_class,skip) base_##enum_name = (uint32_t)parent_class::ID_LAST_##parent_class + skip,
#define BEGINE_EVENTS_INTERNAL(enum_name) base_##enum_name = 20000,
#define EVENT_DECL(enum_name,name,cnt) name = base_##enum_name+cnt+1,
#define END_EVENTS(enum_name,cnt) ID_LAST_##enum_name=base_##enum_name+cnt+1

enum CommonTypes : uint32_t
{
    evFinish=0,  // this event will finish the Processor that receives it
    evConnect=1, //! on the link level this means a new connection, higher level handlers are also notified by this event
    evDisconnect=2,
    evTimeout=3,
    ID_LAST_CommonTypes
};

class Event
{
protected:
        const uint32_t  m_type;
        EventSrc *m_event_source;
        std::atomic<int> m_ref_count {1}; // used to prevent event being deleted when it's in multiple queues

public:

virtual                 ~Event()
                        {
                            // we allow delete when there is 1 reference left (static variables on exit)
                            assert(m_ref_count<=1);
                            m_event_source=nullptr;
                        }
                        Event(uint32_t evtype,EventSrc *ev_src=nullptr) :
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
        void            src(EventSrc *ev_src) {m_event_source=ev_src;}
        EventSrc *      src() const {return m_event_source;}
        uint32_t        type() const {return m_type;}
virtual const char *    info();

protected:
virtual void            do_serialize(std::ostream &os)  = 0;
};
#define EVENT_IMPL(name)\
    template<class Archive>\
    void serialize(Archive & archive); \
    void do_serialize(std::ostream &os) override {\
        cereal::BinaryOutputArchive oarchive(os);\
        oarchive(*this);\
    }\
    ~name() override = default;


// [[ev_def:type]]
class Timeout final: public Event
{
public:
                        // [[ev_def:field]]
    ACE_Time_Value          m_arrival_time;
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
// [[ev_def:type]]
struct Finish final: public Event
{
public:
                    Finish(EventSrc *source=nullptr) : Event(evFinish,source) {}
static  Finish *    s_instance;
        EVENT_IMPL(Finish)
};
} // end of SEGSEventsNamespace
