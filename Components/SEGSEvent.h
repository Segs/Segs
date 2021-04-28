/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/CompiletimeHash.h"

#include "cereal/archives/memory_binary.hpp"
#include <ace/Time_Value.h>
#include <atomic>
#include <cassert>
#include <typeinfo>

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
        std::atomic<int> m_ref_count {1}; // used to prevent event being deleted when it's in multiple queues
        EventSrc *      m_event_source;

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
// Note those are friend functions that will store/restore this message in the std::stream.
friend Event *          from_storage(const std::vector<uint8_t> &istr);
friend void             to_storage(std::vector<uint8_t> &ostr,Event *ev);

virtual void            do_serialize(std::vector<uint8_t> &os)  = 0;
virtual void            serialize_from(const std::vector<uint8_t> &os)  = 0;
};
#define EVENT_IMPL(name)\
    template<class Archive>\
    void serialize(Archive & archive); \
    void do_serialize(std::vector<uint8_t> &os) override {\
        cereal::VectorOutputArchive oarchive(os);\
        oarchive(*this);\
    }\
    void serialize_from(const std::vector<uint8_t> &os) override {\
        cereal::VectorInputArchive iarchive(os);\
        iarchive(*this);\
    }\
    ~name() override = default;


// [[ev_def:type]]
struct Finish final: public Event
{
public:
                    Finish(EventSrc *source=nullptr) : Event(evFinish,source) {}
static  Finish *    s_instance;
        EVENT_IMPL(Finish)
};
} // end of SEGSEventsNamespace
