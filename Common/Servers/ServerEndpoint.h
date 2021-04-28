/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/EventProcessor.h"

#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Dgram.h>
#include <ace/Reactor_Notification_Strategy.h>

#include <unordered_map>

namespace std
{
    // std::hash override for ACE_INET_Addr
    template<>
    class hash<ACE_INET_Addr>
    {
    public:
            size_t operator()( const ACE_INET_Addr& Key ) const
            {
                std::hash<ACE_UINT32> hasher;
                return hasher(Key.get_ip_address()) ^ (hasher(Key.get_port_number()<<8));
            }
    };
}


class CRUDLink;

// This class represents a UDP port<->Client link router
// when it receives any bytes it will pass them to appropriate ILink instance
// when it receives PacketEvents from ILink it sends the bytes over the UDP to associated address
class ServerEndpoint : public EventSrc
{
    typedef EventSrc super;
    typedef std::unordered_map<ACE_INET_Addr,CRUDLink *> hmAddrProto;
public:

                    ServerEndpoint(const ACE_INET_Addr &local_addr) :
                        m_notifier(nullptr, nullptr, ACE_Event_Handler::WRITE_MASK),
                        endpoint_ (local_addr), m_downstream(nullptr)
                    {
                        m_notifier.event_handler(this);
                    }
                    ~ServerEndpoint() override;
private:
        // Part of the low level ace interface, not passed on to derived classes
        ACE_HANDLE  get_handle(void) const override { return this->endpoint_.get_handle(); }
        int         handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE) override;
        int         handle_output (ACE_HANDLE fd = ACE_INVALID_HANDLE) override;
        int         handle_close (ACE_HANDLE /*handle*/,ACE_Reactor_Mask /*close_mask*/) override;
public:
        int         open(void *p=nullptr) override;
        void        set_downstream(EventProcessor *ds) { m_downstream = ds;}
protected:
        CRUDLink *  createLinkInstance();
        CRUDLink *  getClientLink(const ACE_INET_Addr &from_addr);
virtual CRUDLink *  createLink(EventProcessor *down) = 0;

        hmAddrProto client_links;
        ACE_Reactor_Notification_Strategy m_notifier;
        ACE_SOCK_Dgram endpoint_;   // Wrapper for sending/receiving dgrams.
        ACE_Thread_Mutex m_send_sema;
        EventProcessor *m_downstream; //!< All created links will have this as their downstream target
};

struct ListenAndLocationAddresses
{
    ACE_INET_Addr m_listen_addr;
    ACE_INET_Addr m_location_addr;
    ListenAndLocationAddresses(ACE_INET_Addr listen,ACE_INET_Addr location,uint16_t inc=0)
    {
        m_listen_addr = listen;
        m_location_addr = location;
        m_listen_addr.set_port_number(listen.get_port_number()+inc);
        m_location_addr.set_port_number(location.get_port_number()+inc);
    }
    ListenAndLocationAddresses(const ListenAndLocationAddresses &oth,uint16_t inc=0)
    {
        m_listen_addr = oth.m_listen_addr;
        m_location_addr = oth.m_location_addr;
        m_listen_addr.set_port_number(oth.m_listen_addr.get_port_number()+inc);
        m_location_addr.set_port_number(oth.m_location_addr.get_port_number()+inc);
    }
};
