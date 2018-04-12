/*
 * Super Entity Game Server
 * http://github.com/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "Common/CRUDP_Protocol/ILink.h"
#include "InternalEvents.h"
#include "SEGSEvent.h"

#include <ace/INET_Addr.h>
#include <ace/OS_NS_time.h>
#include <ace/Synch.h>
#include <unordered_map>
#include <vector>
#include <cassert>


// TODO: consider storing all sessions in vector, and convert m_session_store to token->index map
template <class SESSION_CLASS>
class ClientSessionStore
{
public:
using   MTGuard = ACE_Guard<ACE_Thread_Mutex>;
using   vClients = std::vector<SESSION_CLASS *>;
using   ivClients = typename vClients::iterator;
using   civClients = typename vClients::const_iterator;
protected:
        mutable ACE_Thread_Mutex m_store_mutex;
        struct ExpectClientInfo
        {
            uint32_t cookie;
            ACE_Time_Value m_expected_since;
            uint64_t session_token;
        };
        std::unordered_map<uint64_t,SESSION_CLASS> m_token_to_session;
        std::unordered_map<uint32_t,uint64_t> m_id_to_token;
        std::vector<ExpectClientInfo> m_session_expecting_clients;
        vClients m_active_sessions;
        uint32_t create_cookie(const ACE_INET_Addr &from,uint64_t id)
        {
                uint64_t res = ((from.hash()+id)&0xFFFFFFFF)^(id>>32);
                ACE_DEBUG ((LM_WARNING,ACE_TEXT ("(%P|%t) create_cookie still needs a good algorithm.0x%08x\n"),res));
                return (uint32_t)res;
        }

public:
        ivClients       begin() { return m_active_sessions.begin();}
        ivClients       end() { return m_active_sessions.end();}
        civClients      begin() const { return m_active_sessions.cbegin();}
        civClients      end() const { return m_active_sessions.cend();}
        ACE_Thread_Mutex &store_lock() { return m_store_mutex; }
        SESSION_CLASS &createSession(uint64_t token)
        {
            assert(m_token_to_session.find(token)==m_token_to_session.end());
            return m_token_to_session[token];
        }
        uint64_t tokenForId(uint32_t id) const
        {
            auto iter = m_id_to_token.find(id);
            if(iter==m_id_to_token.end())
                return 0;
            return iter->second;
        }
        void setTokenForId(uint32_t id,uint64_t token)
        {
            m_id_to_token[id] = token;
        }
        bool hasSessionFor(uint64_t token) const
        {
            return m_token_to_session.find(token)!=m_token_to_session.end();
        }
        SESSION_CLASS &sessionFromToken(uint64_t token)
        {
            auto iter = m_token_to_session.find(token);
            assert(iter != m_token_to_session.end());
            return iter->second;
        }
        SESSION_CLASS &sessionFromEvent(SEGSEvent *ev)
        {
            assert(dynamic_cast<LinkBase *>(ev->src())!=nullptr); // make sure the event source is a Link
            LinkBase * lnk = (LinkBase *)ev->src();
            auto iter = m_token_to_session.find(lnk->session_token());
            assert(iter!=m_token_to_session.end());
            SESSION_CLASS &session(iter->second);
            assert(session.m_link==lnk);
            return session;
        }

        SESSION_CLASS &sessionFromEvent(InternalEvent *ev)
        {
            auto iter = m_token_to_session.find(ev->session_token());
            assert(iter != m_token_to_session.end());
            return iter->second;
        }
        uint32_t ExpectClientSession(uint64_t token,const ACE_INET_Addr &from,uint64_t id)
        {
                uint32_t cook = create_cookie(from,id);
                for(ExpectClientInfo sess : m_session_expecting_clients )
                {
                // if we already expect this client
                    if(sess.cookie==cook)
                {
                        assert(false);
                        // return pregenerated cookie
                        return cook;
                }
                }
                m_session_expecting_clients.emplace_back(ExpectClientInfo{cook,ACE_OS::gettimeofday(),token});
                return cook;
        }
        void sessionLinkLost(uint64_t token)
        {
            SESSION_CLASS &session(sessionFromToken(token));
            removeFromActiveSessions(&session);
            for (size_t idx = 0, total = m_session_expecting_clients.size(); idx < total; ++idx)
            {
                if (m_session_expecting_clients[idx].session_token == token)
                {
                    std::swap(m_session_expecting_clients[idx], m_session_expecting_clients.back());
                    m_session_expecting_clients.pop_back();
                    break;
                }
            }
            session.m_link = nullptr;
        }
        void removeByToken(uint64_t token,uint32_t id)
        {
            m_id_to_token.erase(id);
            m_token_to_session.erase(token);
        }
        uint64_t connectedClient(uint32_t cookie)
        {
            for (size_t idx = 0, total = m_session_expecting_clients.size(); idx < total; ++idx)
            {
                if (m_session_expecting_clients[idx].cookie == cookie)
                {
                    uint64_t expected_in_session = m_session_expecting_clients[idx].session_token;
                    std::swap(m_session_expecting_clients[idx], m_session_expecting_clients.back());
                    m_session_expecting_clients.pop_back();
                    return expected_in_session;
                }
            }
            return ~0U;
        }
        void removeFromActiveSessions(SESSION_CLASS *cl)
        {
            for(size_t idx=0,total=m_active_sessions.size(); idx<total; ++idx)
            {
                if(m_active_sessions[idx]==cl)
                {
                    std::swap(m_active_sessions[idx],m_active_sessions.back());
                    m_active_sessions.pop_back();
                    return;
                }
            }
            assert(false);
        }
        void addToActiveSessions(SESSION_CLASS *cl)
        {
            for(size_t idx=0,total=m_active_sessions.size(); idx<total; ++idx)
            {
                if(m_active_sessions[idx]==cl)
                    return;
            }
            m_active_sessions.emplace_back(cl);
        }
        bool isActive(SESSION_CLASS *c) const
        {
            return std::find(m_active_sessions.begin(),m_active_sessions.end(),c)!=m_active_sessions.end();
        }
        size_t num_active_clients() const
        {
            return m_active_sessions.size();
        }
};
