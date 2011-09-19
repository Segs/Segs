#pragma once
#include <ace/INET_Addr.h>
#include "SEGSEvent.h"

template<class BUFFER>
class SerializableEvent : public SEGSEvent
{
protected:
    virtual ~SerializableEvent() {}
public:
        SerializableEvent(size_t evtype,EventProcessor *ev_src=0) : SEGSEvent(evtype,ev_src)
        {}
        virtual void serializeto(BUFFER &) const=0;
        virtual void serializefrom(BUFFER &)=0;
};
class ConnectEvent : public SEGSEvent
{
public:
    ConnectEvent(EventProcessor *ev_src) : SEGSEvent(SEGS_EventTypes::evConnect,ev_src)
    {
        ACE_ASSERT(ev_src);
    }
    ConnectEvent(EventProcessor *ev_src,const ACE_INET_Addr &addr) :
        SEGSEvent(SEGS_EventTypes::evConnect,ev_src),
        src_addr(addr)
    {
        ACE_ASSERT(ev_src);
    }
    ACE_INET_Addr src_addr;
};
class DisconnectEvent : public SEGSEvent
{
public:
    DisconnectEvent(EventProcessor *ev_src) : SEGSEvent(SEGS_EventTypes::evDisconnect,ev_src)
    {
        ACE_ASSERT(ev_src);
    }
};
