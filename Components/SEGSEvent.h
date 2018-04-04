#pragma once
#include <ace/Time_Value.h>
#include <typeinfo>
class EventProcessor;

// Helper defines to ease the definition of event types
#define INIT_EVENTS() enum { base = 0,
#define BEGINE_EVENTS(parent_class) enum { base = parent_class::evLAST_EVENT,
#define BEGINE_EVENTS_INTERNAL() enum { base = 2000,
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
        const size_t    m_type;
        EventProcessor *m_event_source;

virtual                 ~SEGSEvent()
                        {
                            m_event_source=nullptr;
                        }
public:
                        SEGSEvent(size_t evtype,EventProcessor *ev_src=nullptr) : m_type(evtype),m_event_source(ev_src)
                        {}
        void            release()
                        {
                            delete this;
                        }
        void            src(EventProcessor *ev_src) {m_event_source=ev_src;}
        EventProcessor *src() {return m_event_source;}
        size_t          type() const {return m_type;}
virtual const char *    info() {return typeid(*this).name();}
};

class TimerEvent : public SEGSEvent
{
    ACE_Time_Value          m_arrival_time;
    void *                  m_data;

public:
                            TimerEvent(const ACE_Time_Value &time, void *dat)
                                : SEGSEvent(SEGS_EventTypes::evTimeout), m_arrival_time(time), m_data(dat)
                            {
                            }
    void *                  data() { return m_data; }
    const ACE_Time_Value &  arrival_time() { return m_arrival_time; }
};
