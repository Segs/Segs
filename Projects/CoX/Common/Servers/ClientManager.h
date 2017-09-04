/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "Client.h"
#include "AdminServer/AccountInfo.h"

#include <unordered_map>
#include <unordered_set>
#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch.h>


template <class CLIENT_CLASS>
class ClientStore
{
public:
typedef std::unordered_set<CLIENT_CLASS *> vClients;
typedef typename vClients::iterator          ivClients;
protected:
        //  boost::object_pool<CLIENT_CLASS> m_pool;
        std::unordered_map<uint32_t,CLIENT_CLASS *> m_expected_clients;
        std::unordered_map<uint64_t,CLIENT_CLASS *> m_clients; // this maps client's id to it's object
        std::unordered_map<uint32_t,CLIENT_CLASS *> m_connected_clients_cookie; // this maps client's id to it's object
        std::unordered_map<uint64_t,uint32_t> m_id_to_cookie; // client cookie is only useful in this context
        vClients m_active_clients;
        uint32_t create_cookie(const ACE_INET_Addr &from,uint64_t id)
        {
                uint64_t res = ((from.hash()+id)&0xFFFFFFFF)^(id>>32);
                ACE_DEBUG ((LM_WARNING,ACE_TEXT ("(%P|%t) create_cookie still needs a good algorithm.0x%08x\n"),res));
                return (uint32_t)res;
        }

public:
        ivClients       begin() { return m_active_clients.begin();}
        ivClients       end() { return m_active_clients.end();}

        CLIENT_CLASS *getById(uint64_t id)
        {
                if(m_clients.find(id)==m_clients.end())
                        return NULL;
                return m_clients[id];
        }

        CLIENT_CLASS *getByCookie(uint32_t cookie)
        {
                if(m_connected_clients_cookie.find(cookie)!=m_connected_clients_cookie.end())
                {
                        return m_connected_clients_cookie[cookie];
                }
                return NULL;
        }

        CLIENT_CLASS *getExpectedByCookie(uint32_t cookie)
        {
                // we got cookie check if it's an expected client
                if(m_expected_clients.find(cookie)!=m_expected_clients.end())
                {
                        return m_expected_clients[cookie];
                }
                return NULL;
        }

        uint32_t ExpectClient(const ACE_INET_Addr &from,uint64_t id,uint8_t access_level)
        {
                CLIENT_CLASS * exp;
                uint32_t cook = create_cookie(from,id);
                // if we already expect this client
                if(m_expected_clients.find(cook)!=m_expected_clients.end())
                {
                        // return pregenerated cookie
                        return cook;
                }
                if(getByCookie(cook)) // already connected ?!
                {
                        //
                        return ~0U; // invalid cookie
                }
                exp = new CLIENT_CLASS;
                exp->account_info().access_level(access_level);
                exp->account_info().account_server_id(id);
                exp->link_state().setState(ClientLinkState::CLIENT_EXPECTED);
                m_expected_clients[cook] = exp;
                m_clients[id] = exp;
                m_id_to_cookie[id]=cook;
                return cook;
        }
        void removeById(uint64_t id)
        {
                ACE_ASSERT(getById(id)!=0);
                CLIENT_CLASS *cl=getById(id);
                uint32_t cookie = m_id_to_cookie[id];
                m_active_clients.erase(cl);
                m_id_to_cookie.erase(id);
                m_expected_clients.erase(cookie);
                m_connected_clients_cookie.erase(cookie);
                m_clients.erase(id);

                delete cl;
        }
        void connectedClient(uint32_t cookie)
        {
                // client with cookie has just connected
                m_connected_clients_cookie[cookie]=getExpectedByCookie(cookie);
                // so he's no longer expected
                m_expected_clients.erase(cookie);
        }

        void addToActiveClients(CLIENT_CLASS *cl) {
            m_active_clients.insert(cl);
        }
        size_t num_active_clients() const {
            return m_active_clients.size();
        }
};
