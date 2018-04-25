/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include "EventProcessor.h"
#include <cassert>

class BitStream;
class LinkBase : public EventProcessor
{
            // initialized to unknown session
            uint64_t        m_session_token=0;  //!< Handler-unique number identifying client's session
public:
virtual                     ~LinkBase() = default;
            uint64_t        session_token() const { return m_session_token; }
            void            session_token(uint64_t tok) { m_session_token=tok; assert(m_session_token!=0);}
};

