#pragma once

#include "EventProcessor.h"
#include <cassert>

class BitStream;
class ILink : public EventProcessor
{
    uint64_t        m_session_token;  //!< Handler-unique number identifying client's session
public:
    virtual ~ILink() = default;
    uint64_t        session_token() const { assert(m_session_token!=0); return m_session_token; }
    void            session_token(uint64_t tok) { m_session_token=tok; assert(m_session_token!=0);}
};

