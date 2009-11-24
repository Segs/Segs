#pragma once
#include <ace/INET_Addr.h>
#include "SEGSEvent.h"

template<class BUFFER,class LINK>
class LinkLevelEvent : public SEGSEvent
{
protected:
    virtual ~LinkLevelEvent() {}
public:
	LinkLevelEvent(size_t evtype,EventProcessor *ev_src=0) : SEGSEvent(evtype,ev_src)
	{}
	virtual void serializeto(BUFFER &) const=0;
	virtual void serializefrom(BUFFER &)=0;
};
class ConnectEvent : public SEGSEvent
{
public:
    ConnectEvent(EventProcessor *ev_src) : SEGSEvent(evConnect,ev_src)
    {
        ACE_ASSERT(ev_src);
    }
    ConnectEvent(EventProcessor *ev_src,const ACE_INET_Addr &addr) : SEGSEvent(evConnect,ev_src),src_addr(addr)
    {
        ACE_ASSERT(ev_src);
    }
    ACE_INET_Addr src_addr;
};
class DisconnectEvent : public SEGSEvent
{
public:
    DisconnectEvent(EventProcessor *ev_src) : SEGSEvent(evDisconnect,ev_src)
    {
        ACE_ASSERT(ev_src);
    }
};
