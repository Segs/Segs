#pragma once
#include <ace/Time_Value.h>
#include <atomic>
#include <typeinfo>
#include <cassert>
class EventProcessor;

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

class SEGSEvent
{
protected:
        const uint32_t  m_type;
        EventProcessor *m_event_source;
        std::atomic<int> m_ref_count {1}; // used to prevent event being deleted when it's in multiple queues

public:

virtual                 ~SEGSEvent()
                        {
                            // we allow delete when there is 1 reference left (static variables on exit)
                            assert(m_ref_count<=1);
                            m_event_source=nullptr;
                        }
                        SEGSEvent(uint32_t evtype,EventProcessor *ev_src=nullptr) :
                            m_type(evtype),m_event_source(ev_src)
                        {}
        SEGSEvent *     shallow_copy() // just incrementing the ref count
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
        EventProcessor *src() {return m_event_source;}
        uint32_t        type() const {return m_type;}
virtual const char *    info();

static  SEGSEvent       s_ev_finish;
};

class TimerEvent final: public SEGSEvent
{
    ACE_Time_Value          m_arrival_time;
    void *                  m_data;

public:
                            TimerEvent(const ACE_Time_Value &time, void *dat,EventProcessor *source)
                                : SEGSEvent(SEGS_EventTypes::evTimeout,source), m_arrival_time(time), m_data(dat)
                            {
                            }
    void *                  data() { return m_data; }
    const ACE_Time_Value &  arrival_time() { return m_arrival_time; }
};
