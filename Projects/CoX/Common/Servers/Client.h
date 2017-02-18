/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <string>
#include <ace/INET_Addr.h>
//#include "ServerManager.h"
class QString;
class Entity;
class EventProcessor;
class AccountInfo;
class ClientLinkState
{
public:
    typedef enum
    {
        CLIENT_DISCONNECTED=0,
        CLIENT_EXPECTED,
        NOT_LOGGED_IN,
        LOGGED_IN,
        CLIENT_CONNECTED
    } eClientState;
public:
                        ClientLinkState():m_state(NOT_LOGGED_IN){}
    uint32_t            hash_id() {return m_peer_addr.get_ip_address()^m_peer_addr.get_port_number();}
    void                setState(eClientState s) {m_state=s;}
    eClientState        getState() {return m_state;}
    void                setPeer(const ACE_INET_Addr &peer){m_peer_addr=peer;}
    ACE_INET_Addr &     getPeer() {return m_peer_addr;}
    EventProcessor *    link() const { return m_link; }
    void                link(EventProcessor * val) { m_link = val; }

protected:
    ACE_INET_Addr       m_peer_addr;
    eClientState        m_state;
    EventProcessor *    m_link;
};
class ClientSession
{
public:
    ClientSession();
    virtual ~ClientSession();
    QString             getLogin()  const;
    bool                account_blocked();
    AccountInfo &       account_info()      { return *m_account_info;}
    ClientLinkState &   link_state()        { return m_link;}
protected:
    ClientLinkState     m_link;
    AccountInfo         *m_account_info;
};
