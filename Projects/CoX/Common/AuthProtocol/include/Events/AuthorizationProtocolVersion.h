#pragma once
#include "AuthEvents.h"

class AuthorizationProtocolVersion : public AuthLinkEvent
{
	u32		m_seed;
	u32		m_proto_vers;
public:
	AuthorizationProtocolVersion() : AuthLinkEvent(evAuthProtocolVersion),m_seed(1),m_proto_vers(0)
	{}
	AuthorizationProtocolVersion(EventProcessor *ev_src,u32 version_id,u32 seed) :
		AuthLinkEvent(evAuthProtocolVersion,ev_src),
		m_seed(seed),
		m_proto_vers(version_id)
	{}
	void init(EventProcessor *ev_src,u32 version_id,u32 seed) {m_proto_vers=version_id;m_event_source=ev_src; m_seed=seed;}
	void serializeto(GrowingBuffer &buf) const
	{
		buf.uPut((u8)0);
		buf.uPut(m_seed);
		buf.uPut(m_proto_vers);
	}
	void serializefrom(GrowingBuffer &buf)
	{
		u8 packet_type;
		buf.uGet(packet_type);
		buf.uGet(m_seed);
		buf.uGet(m_proto_vers);
	}
};
