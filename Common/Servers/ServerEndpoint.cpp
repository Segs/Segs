/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup CommonServers Projects/CoX/Common/Servers
 * @{
 */

#include "ServerEndpoint.h"

#include "Common/CRUDP_Protocol/CRUD_Events.h"
#include "Common/CRUDP_Protocol/CRUD_Link.h"
#include "Components/BitStream.h"

#include <ace/Reactor.h>
#include <ace/Message_Block.h>

using namespace SEGSEvents;

ServerEndpoint::~ServerEndpoint() 
{
    handle_close(ACE_INVALID_HANDLE, 0);
    m_downstream = nullptr;
}

int ServerEndpoint::handle_input(ACE_HANDLE /*fd*/) //! Called when input is available from the client.
{
    uint8_t buf[0x2000];
    ACE_INET_Addr from_addr;
    ssize_t n = this->endpoint_.recv(buf, sizeof buf,from_addr);
    if(n == -1)
        ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("%p\n"),"handle_input"),0);
    CRUDLink *crud_link = getClientLink(from_addr); // get packet handling object for this connection
    ACE_ASSERT(crud_link!=nullptr);
    BitStream wrap((uint8_t *)buf,n);
    crud_link->received_block(wrap);
    return 0;
}

int ServerEndpoint::handle_output(ACE_HANDLE /*fd*/) //! Called when output is possible.
{
    Event *ev;
    ACE_Time_Value nowait (ACE_OS::gettimeofday ());
    while (-1 != getq(ev, &nowait))
    {
        if(ev->type()==evFinish)
        {
            ACE_ASSERT(!"Post finish message to all links");
        }
        else if(ev->type()==CRUD_EventTypes::evPacket)
        {
            Packet *pkt_ev = (Packet *)ev;
            ssize_t send_cnt = endpoint_.send(pkt_ev->bytes(),pkt_ev->size(),pkt_ev->target);
            if(send_cnt == -1)
            {
                // inform the link that it should die.
                pkt_ev->src()->putq(Finish::s_instance->shallow_copy());
                ACE_ERROR ((LM_ERROR,ACE_TEXT ("(%P|%t) %p\n"), ACE_TEXT ("send")));
                ev->release();
                break;
            }
        }
        ev->release();
    }
    if(msg_queue()->is_empty ()) // we don't want to be woken up
        reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    else // unless there is something to send still
        reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    return 0;
}

//! Called when this handler is removed from the ACE_Reactor.
int ServerEndpoint::handle_close(ACE_HANDLE, ACE_Reactor_Mask)
{
    if(this->reactor() && this->reactor()->remove_handler(this, ALL_EVENTS_MASK | DONT_CALL) == -1)
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("unable to unregister client handler")), -1);
    endpoint_.close();
    return 0;
}

int ServerEndpoint::open(void *p)
{
    if(super::open (p) == -1)
        return -1;
    // register ourselves with the reactor,
    // it will call our handle_input method whenever there are
    // new bytes available on our handle.
    if(this->reactor () && this->reactor ()->register_handler(this,ACE_Event_Handler::READ_MASK) == -1)
        ACE_ERROR_RETURN ((LM_ERROR,ACE_TEXT ("%p\n"),ACE_TEXT ("unable to register client handler")),-1);

    m_notifier.reactor(reactor()); // notify current reactor with write event,
    msg_queue()->notification_strategy (&m_notifier); // whenever there is a new msg on msg_queue()
    return 0;
}

CRUDLink *ServerEndpoint::createLinkInstance()
{
    CRUDLink *res = createLink(m_downstream); // create a new client handler
    res->reactor(reactor());
    if(-1==res->open())
    {
        delete res;
        return nullptr;
    }
    return res;
}

CRUDLink *ServerEndpoint::getClientLink(const ACE_INET_Addr &from_addr)
{
    CRUDLink *res= client_links[from_addr]; // get packet handling object for this connection
    if(res!=nullptr)
        return res;

    res = createLinkInstance(); // create a new client handler
    if(nullptr == res)
        return nullptr;
    res->putq(new Connect(this,from_addr)); // and inform it of a new connection
    client_links[from_addr] = res;
    //TODO: schedule timeout timer here!!
    return res;
}

//! @}
