/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once

#include <string>
#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Dgram.h>
#include <ace/Message_Queue.h>
#include <ace/Message_Block.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>
#include <ace/Reactor_Notification_Strategy.h>
#include "EventProcessor.h"
#include "CRUD_Events.h"
#include "Base.h"
#include "hashmap_selector.h"
#ifndef WIN32
class ACE_INET_Addr_Hash
{
public:
        ACE_INET_Addr_Hash( ){}
        //hash_compare( Traits pred );
        size_t operator( )( const ACE_INET_Addr& Key ) const
        {
            return Key.get_ip_address()^(Key.get_port_number()<<8);
        }
};
#else
class ACE_INET_Addr_Hash : public hash_compare<ACE_INET_Addr>
{
public:
        ACE_INET_Addr_Hash( ){};
        //hash_compare( Traits pred );
        size_t operator( )( const ACE_INET_Addr& Key ) const
        {
                return Key.get_ip_address()^(Key.get_port_number()<<8);
        };
        bool operator()( const ACE_INET_Addr& _Key1, const ACE_INET_Addr& _Key2 ) const
        {
                return _Key1 < _Key2;
        }
};
#endif // WIN32



// This class represents a UDP packet router
// when it receives any bytes it will pass them to appropriate LINK
// when it receives PacketEvent's it sends the bytes over the UDP to target address
template<class LINK_CLASS>
class ServerEndpoint : public EventProcessor
{
    typedef EventProcessor super;
    typedef hash_map<ACE_INET_Addr,LINK_CLASS *,ACE_INET_Addr_Hash> hmAddrProto;
public:
    typedef ACE_SOCK_Dgram stream_type;
    typedef ACE_INET_Addr addr_type;

                    ServerEndpoint(const ACE_INET_Addr &local_addr) :
                        m_notifier(0, 0, ACE_Event_Handler::WRITE_MASK),
                        endpoint_ (local_addr)
                    {
                        m_notifier.event_handler(this);
                    }
private:
        // Part of the low level ace interface, not passed on to derived classes
        ACE_HANDLE  get_handle(void) const { return this->endpoint_.get_handle(); }

        int         handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE) //! Called when input is available from the client.
                    {
                        (void)fd;
                        uint8_t buf[0x2000];
                        ACE_INET_Addr from_addr;
                        ssize_t n = this->endpoint_.recv(buf, sizeof buf,from_addr);
                        if (n == -1)
                            ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("%p\n"),"handle_input"),0);
                        LINK_CLASS *crud_link = getClientLink(from_addr); // get packet handling object for this connection
                        ACE_ASSERT(crud_link!=NULL);
                        BitStream wrap((uint8_t *)buf,n);
                        crud_link->received_block((BitStream &)wrap);
                        return 0;
                    }

        int         handle_output (ACE_HANDLE fd = ACE_INVALID_HANDLE) //! Called when output is possible.
                    {
                        (void)fd;
                        SEGSEvent *ev;
                        ACE_Time_Value nowait (ACE_OS::gettimeofday ());
                        while (-1 != getq(ev, &nowait))
                        {
                            if(ev->type()==SEGS_EventTypes::evFinish)
                            {
                                ACE_ASSERT(!"Post finish message to all links");
                            }
                            else if(ev->type()==CRUD_EventTypes::evPacket)
                            {
                                PacketEvent *pkt_ev = (PacketEvent *)ev;
                                ssize_t send_cnt = endpoint_.send(pkt_ev->bytes(),pkt_ev->size(),pkt_ev->target);
                                if (send_cnt == -1)
                                {
                                    // inform the link that it should die.
                                    pkt_ev->src()->putq(new SEGSEvent(SEGS_EventTypes::evFinish));
                                    ACE_ERROR ((LM_ERROR,ACE_TEXT ("(%P|%t) %p\n"),
                                                ACE_TEXT ("send")));
                                    ev->release();
                                    break;
                                }
                            }
                            ev->release();
                        }
                        if (msg_queue()->is_empty ()) // we don't want to be woken up
                            reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
                        else // unless there is something to send still
                            reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
                        return 0;
                    }

                                //! Called when this handler is removed from the ACE_Reactor.
        int         handle_close (ACE_HANDLE /*handle*/,ACE_Reactor_Mask /*close_mask*/)
                    {
                        endpoint_.close();
                        return 0;
                    }
public:
        int         open(void *p=NULL)
                    {
                        if (super::open (p) == -1)
                            return -1;
                        // register ourselves with the reactor,
                        // it will call our handle_input method whenever there are
                        // new bytes available on our handle.
                        if (this->reactor () && this->reactor ()->register_handler(this,ACE_Event_Handler::READ_MASK) == -1)
                            ACE_ERROR_RETURN ((LM_ERROR,ACE_TEXT ("%p\n"),ACE_TEXT ("unable to register client handler")),-1);

                        m_notifier.reactor(reactor()); // notify current reactor with write event,
                        msg_queue()->notification_strategy (&m_notifier); // whenever there is a new msg on msg_queue()
                        return 0;
                    }
protected:
        void        dispatch(SEGSEvent *)
                    {
                        ACE_ASSERT(!"All events are dispatched from handle_* methods");
                    }
        SEGSEvent * dispatch_sync( SEGSEvent *)
                    {
                        ACE_ASSERT(!"No sync events known");
                        return 0;
                    }
        LINK_CLASS *getClientLink(const ACE_INET_Addr &from_addr)
                    {
                        LINK_CLASS *res= client_links[from_addr]; // get packet handling object for this connection
                        if(res!=NULL)
                            return res;

                        res = new LINK_CLASS; // create a new client handler
                        res->reactor(reactor());
                        if(-1==res->open())
                        {
                            delete res;
                            return 0;
                        }
                        res->putq(new ConnectEvent(this,from_addr)); // and inform it of a new connection
                        client_links[from_addr]	= res;
                        // schedule timeout timer here!!
                        return res;
                    }

        hmAddrProto client_links;
        ACE_Reactor_Notification_Strategy m_notifier;
        ACE_SOCK_Dgram endpoint_;	// Wrapper for sending/receiving dgrams.
        ACE_Thread_Mutex m_send_sema;
};
