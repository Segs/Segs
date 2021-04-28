/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/CRUDP_Protocol/ILink.h"
#include "InternalEvents.h"
#include "Components/SEGSEvent.h"
#include "Components/SEGSTimer.h"
#include "Components/Logging.h"

#include <ace/OS_NS_time.h>

#include <QDebug>
#include <thread>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <vector>
#include <cassert>
#include <cinttypes>

/**
 * @brief The ClientSession struct is a base for all class for all sessions registered in ClientSessionStore
 */
struct ClientSession
{
        uint64_t    m_session_token= 0; // a back-link to owning session, used in serialization
};

#define DEBUG_REAPING

#ifdef DEBUG_REAPING
struct  ReapingMetadata
{
        //! @note This class does not take ownership of the `const char *` memory
        //! the best approach is to pass it '.data' section strings only.
        std::unordered_map<uint64_t,const char *> m_reaped_link_reason;
        void marked_for_reaping(uint64_t token,const char *reason)
        {
            m_reaped_link_reason[token] = reason;
        }
        void unmarked_for_reaping(uint64_t token)
        {
            m_reaped_link_reason.erase(token);
        }
        [[noreturn]]
        void fatal_failure(const char *msg,uint64_t token)
        {
            auto iter = m_reaped_link_reason.find(token);
            const char *reason = "Unregistered reason";
            if(iter!=m_reaped_link_reason.end())
                reason = iter->second;
            qFatal("Session failure %s : Reaped because: %s",msg,reason);
        }

};
#else
struct  ReapingMetadata
{
        void marked_for_reaping(uint64_t token,const char *reason) {}
        void unmarked_for_reaping(uint64_t token) {}
        [[noreturn]]
        void fatal_failure(const char *msg,uint64_t /*token*/)
        {
            qFatal("Session failure %s",msg);
        }
};
#endif

// TODO: consider storing all sessions in vector, and convert m_session_store to token->index map
template <class SESSION_CLASS>
class ClientSessionStore
{
public:
using   MTGuard = std::lock_guard<std::mutex>;
using   vClients = std::vector<SESSION_CLASS *>;
using   ivClients = typename vClients::iterator;
using   civClients = typename vClients::const_iterator;
protected:

mutable std::mutex m_store_mutex;
        std::mutex m_reaping_mutex;
        struct ExpectClientInfo
        {
            uint32_t m_cookie;
            ACE_Time_Value m_expected_since;
            uint64_t m_session_token;
        };
        ///
        /// \brief The WaitingSession struct is used to store sessions without active connections in any server.
        ///
        struct WaitingSession
        {
            ACE_Time_Value m_waiting_since;
            SESSION_CLASS *m_session;
            uint64_t       m_session_token;
        };
        std::unordered_map<uint64_t,SESSION_CLASS> m_token_to_session;
        std::unordered_map<uint32_t,uint64_t> m_id_to_token;
        std::vector<ExpectClientInfo> m_session_expecting_clients;
        std::vector<WaitingSession> m_session_ready_for_reaping;
        ReapingMetadata m_reaping_data;
        vClients m_active_sessions;
        uint32_t create_cookie(const ACE_INET_Addr &from,uint64_t id)
        {
                uint64_t res = ((from.hash()+id)&0xFFFFFFFF)^(id>>32);
                qWarning("Create_cookie still needs a good algorithm. 0x%" PRIx64, res);
                return uint32_t(res);
        }

public:
        const vClients& get_active_sessions()
        {
            return m_active_sessions;
        }

        ivClients begin()
        {
            return m_active_sessions.begin();
        }
        ivClients end()
        {
            return m_active_sessions.end();
        }
        civClients begin() const
        {
            return m_active_sessions.cbegin();
        }
        civClients end() const
        {
            return m_active_sessions.cend();
        }
        std::mutex &store_lock()
        {
            return m_store_mutex;
        }
        std::mutex &reap_lock()
        {
            return m_reaping_mutex;
        }
        SESSION_CLASS &create_session(uint64_t token)
        {
            assert(m_token_to_session.find(token)==m_token_to_session.end());
            SESSION_CLASS &res(m_token_to_session[token]);
            res.m_session_token = token;
            return res;
        }
        uint64_t token_for_id(uint32_t id) const
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
        bool has_session_for(uint64_t token) const
        {
            return m_token_to_session.find(token)!=m_token_to_session.end();
        }
        SESSION_CLASS &session_from_token(uint64_t token)
        {
            auto iter = m_token_to_session.find(token);
            if(iter == m_token_to_session.end())
                m_reaping_data.fatal_failure("missing session",token);
            return iter->second;
        }
        SESSION_CLASS &session_from_event(SEGSEvents::Event *ev)
        {
            assert(dynamic_cast<LinkBase *>(ev->src())!=nullptr); // make sure the event source is a Link
            LinkBase * lnk = static_cast<LinkBase *>(ev->src());
            auto iter = m_token_to_session.find(lnk->session_token());
            if(iter == m_token_to_session.end())
                m_reaping_data.fatal_failure("missing session for event",lnk->session_token());
            SESSION_CLASS &session(iter->second);
            assert(session.link()==lnk);
            return session;
        }

        SESSION_CLASS &session_from_event(SEGSEvents::InternalEvent *ev)
        {
            auto iter = m_token_to_session.find(ev->session_token());
            if(iter == m_token_to_session.end())
                m_reaping_data.fatal_failure("missing session for internal event",ev->session_token());
            return iter->second;
        }
        uint32_t expect_client_session(uint64_t token, const ACE_INET_Addr &from, uint64_t id)
        {
            uint32_t cook = create_cookie(from, id);
            for (const ExpectClientInfo &sess : m_session_expecting_clients)
            {
                // if we already expect this client
                if(sess.m_cookie == cook)
                {
                    // return pregenerated cookie
                    return cook;
                }
            }
            m_session_expecting_clients.emplace_back(ExpectClientInfo{cook, ACE_OS::gettimeofday(), token});
            return cook;
        }
        // used to recover assigned cookie from session
        uint32_t get_cookie_for_session(uint64_t token)
        {
            for (const ExpectClientInfo &sess : m_session_expecting_clients)
            {
                if(sess.m_session_token==token)
                    return sess.m_cookie;
            }
            return 0;
        }
        void session_link_lost(uint64_t token,const char *reason)
        {
            qCDebug(logConnection,"Link lost on session:%lu caused by %s",token,reason);
            SESSION_CLASS &session(session_from_token(token));
            remove_from_active_sessions(&session);
            for (size_t idx = 0, total = m_session_expecting_clients.size(); idx < total; ++idx)
            {
                if(m_session_expecting_clients[idx].m_session_token == token)
                {
                    std::swap(m_session_expecting_clients[idx], m_session_expecting_clients.back());
                    m_session_expecting_clients.pop_back();
                    break;
                }
            }
            session.link(nullptr);
        }
        void remove_by_token(uint64_t token, uint32_t id)
        {
            m_id_to_token.erase(id);
            m_token_to_session.erase(token);
        }
        uint64_t connected_client(uint32_t cookie)
        {
            for (size_t idx = 0, total = m_session_expecting_clients.size(); idx < total; ++idx)
            {
                if(m_session_expecting_clients[idx].m_cookie == cookie)
                {
                    uint64_t expected_in_session = m_session_expecting_clients[idx].m_session_token;
                    std::swap(m_session_expecting_clients[idx], m_session_expecting_clients.back());
                    m_session_expecting_clients.pop_back();
                    return expected_in_session;
                }
            }
            return ~0U;
        }
        void remove_from_active_sessions(SESSION_CLASS *cl)
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
            m_reaping_data.fatal_failure("session not in active sessions",cl->m_session_token);
        }
        void add_to_active_sessions(SESSION_CLASS *cl)
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
        size_t num_sessions() const
        {
            return m_token_to_session.size();
        }
        void mark_session_for_reaping(SESSION_CLASS *sess, uint64_t token,const char *reason)
        {
            m_reaping_data.marked_for_reaping(token,reason);
            m_session_ready_for_reaping.emplace_back(WaitingSession{ACE_OS::gettimeofday(), sess, token});
        }
        void locked_mark_session_for_reaping(SESSION_CLASS *sess, uint64_t token,const char *reason)
        {
            MTGuard guard(reap_lock());
            mark_session_for_reaping(sess,token,reason);
        }

        void unmark_session_for_reaping(SESSION_CLASS *sess)
        {
            m_reaping_data.unmarked_for_reaping(sess->m_session_token);
            for (size_t idx = 0, total = m_session_ready_for_reaping.size(); idx < total; ++idx)
            {
                if(m_session_ready_for_reaping[idx].m_session == sess)
                {
                    std::swap(m_session_ready_for_reaping[idx], m_session_ready_for_reaping.back());
                    m_session_ready_for_reaping.pop_back();
                    break;
                }
            }
        }
        void locked_unmark_session_for_reaping(SESSION_CLASS *sess)
        {
            MTGuard guard(reap_lock());
            unmark_session_for_reaping(sess);
        }
        void reap_stale_links(const char *name, ACE_Time_Value link_is_stale_if_disconnected_for,
                              std::function<void(uint64_t token)> reap_callback = [](uint64_t) {})
        {
            ACE_Time_Value time_now = ACE_OS::gettimeofday();
            for (size_t idx = 0, total = m_session_ready_for_reaping.size(); idx < total; ++idx)
            {
                WaitingSession &waiting_session(m_session_ready_for_reaping[idx]);
                if(time_now - waiting_session.m_waiting_since < link_is_stale_if_disconnected_for)
                    continue;
                //if(waiting_session.m_session->link() == nullptr || waiting_session.m_session->is_temporary() ) // trully disconnected
                {
                    m_reaping_data.unmarked_for_reaping(waiting_session.m_session_token);

                    qCDebug(logConnection) << name << "Reaping stale link" << intptr_t(waiting_session.m_session);
                    reap_callback(waiting_session.m_session_token);
                    if(waiting_session.m_session->link()) // it's a temporary session
                    {
                        // telling the temporary link to close.
                        waiting_session.m_session->link()->putq(SEGSEvents::Finish::s_instance->shallow_copy());
                    }
                    // we destroy the session object
                    remove_by_token(waiting_session.m_session_token, waiting_session.m_session->auth_id());
                    std::swap(m_session_ready_for_reaping[idx], m_session_ready_for_reaping.back());
                    m_session_ready_for_reaping.pop_back();
                    total--; // update the total size
                }
//                else
//                {
//                    // the session still has a link 'active', re-schedule the check
//                    waiting_session.m_waiting_since = time_now;
//                }
            }
        }
        SESSION_CLASS *create_or_reuse_session_for(uint64_t token)
        {
            SESSION_CLASS *     sess;
            MTGuard             guard(reap_lock());

            auto iter = m_token_to_session.find(token);
            if(iter != m_token_to_session.end())
            {
                sess = &iter->second;
                unmark_session_for_reaping(sess);
                qCDebug(logConnection) << "Existing client session reused";
                sess->reset();
            } else
                sess = &create_session(token);
            return sess;
        }
};

/// \note Protecting a session using the ReaperProtection will reset it's liveness-timer
template <class SESSION_CLASS>
struct ReaperProtection
{
    SESSION_CLASS *m_protectee;
    uint64_t m_token;
    ClientSessionStore<SESSION_CLASS> &m_store;
    ReaperProtection(SESSION_CLASS *session,uint64_t token,ClientSessionStore<SESSION_CLASS> &store) :
        m_protectee(session),m_token(token),m_store(store)
    {
        store.unmark_session_for_reaping(session);
        m_protectee=session;
    }
    ~ReaperProtection()
    {
        if(m_protectee)
        {
            typename ClientSessionStore<SESSION_CLASS>::MTGuard guard(m_store.reap_lock());
            m_store.mark_session_for_reaping(m_protectee,m_token,"ReaperProtection: Expired");
        }
    }
    void protectee_moved() { m_protectee = nullptr; }
};
