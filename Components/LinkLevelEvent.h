#pragma once
#include <cassert>
#include <ace/INET_Addr.h>
#include "SEGSEvent.h"


class ConnectEvent : public SEGSEvent
{
public:
    ConnectEvent(EventProcessor *ev_src) : SEGSEvent(SEGS_EventTypes::evConnect,ev_src)
    {
        assert(ev_src);
    }
    ConnectEvent(EventProcessor *ev_src,const ACE_INET_Addr &addr) :
        SEGSEvent(SEGS_EventTypes::evConnect,ev_src),
        src_addr(addr)
    {
        assert(ev_src);
    }
    ACE_INET_Addr src_addr;
};
class DisconnectEvent : public SEGSEvent
{
public:
    DisconnectEvent(EventProcessor *ev_src) : SEGSEvent(SEGS_EventTypes::evDisconnect,ev_src)
    {
        assert(ev_src);
    }
};
