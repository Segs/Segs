/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/Messages/Auth/AuthEvents.h"

namespace SEGSEvents
{

// [[ev_def:type]]
class AuthProtocolVersion : public AuthLinkEvent
{
public:
        // [[ev_def:field]]
        uint32_t        m_seed = 1;
        // [[ev_def:field]]
        uint32_t        m_proto_vers = 0;
                        AuthProtocolVersion() : AuthLinkEvent(evAuthProtocolVersion)
                        {}
                        AuthProtocolVersion(uint32_t version_id,uint32_t seed) :
                            AuthLinkEvent(evAuthProtocolVersion),
                            m_seed(seed),
                            m_proto_vers(version_id)
                        {}
        void            init(EventSrc *ev_src,uint32_t version_id,uint32_t seed)
                        {
                            m_proto_vers=version_id;
                            m_event_source=ev_src; m_seed=seed;
                        }
        void            serializeto(GrowingBuffer &buf) const override
                        {
                            buf.uPut((uint8_t)0);
                            buf.uPut(m_seed);
                            buf.uPut(m_proto_vers);
                        }
        void            serializefrom(GrowingBuffer &buf) override
                        {
                            uint8_t packet_type;
                            buf.uGet(packet_type);
                            buf.uGet(m_seed);
                            buf.uGet(m_proto_vers);
                        }
        uint32_t        getSeed() const
                        {
                            return m_seed;
                        }
        uint32_t        getProtoVers() const
                        {
                            return m_proto_vers;
                        }
        EVENT_IMPL(AuthProtocolVersion)
};
} // end of namespace SEGSEvents
