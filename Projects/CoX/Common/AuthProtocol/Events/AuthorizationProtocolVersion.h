#pragma once
#include "AuthProtocol/AuthEvents.h"

class AuthorizationProtocolVersion : public AuthLinkEvent
{
        uint32_t        m_seed;
        uint32_t        m_proto_vers;
public:
                        AuthorizationProtocolVersion() : AuthLinkEvent(evAuthProtocolVersion),m_seed(1),m_proto_vers(0)
                        {}
                        AuthorizationProtocolVersion(uint32_t version_id,uint32_t seed) :
                            AuthLinkEvent(evAuthProtocolVersion),
                            m_seed(seed),
                            m_proto_vers(version_id)
                        {}
        void            init(EventProcessor *ev_src,uint32_t version_id,uint32_t seed)
                        {
                            m_proto_vers=version_id;
                            m_event_source=ev_src; m_seed=seed;
                        }
        void            serializeto(GrowingBuffer &buf) const
                        {
                            buf.uPut((uint8_t)0);
                            buf.uPut(m_seed);
                            buf.uPut(m_proto_vers);
                        }
        void            serializefrom(GrowingBuffer &buf)
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
};




