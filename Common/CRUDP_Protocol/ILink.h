/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Components/EventProcessor.h"

#include <cassert>

class BitStream;
class LinkBase : public EventSrc
{
            // initialized to unknown session
            uint64_t        m_session_token=0;  //!< Handler-unique number identifying client's session
public:

virtual                     ~LinkBase() override = default;
            uint64_t        session_token() const { return m_session_token; }
            void            session_token(uint64_t tok) { m_session_token=tok; assert(m_session_token!=0);}

            // ACE_Task_Base interface
            int activate(long /*flags*/, int /*n_threads*/, int /*force_active*/, long /*priority*/, int /*grp_id*/,
                         ACE_Task_Base * /*task*/, ACE_hthread_t /*thread_handles*/[], void * /*stack*/ [], size_t /*stack_size*/[],
                         ACE_thread_t /*thread_ids*/[], const char * /*thr_name*/ []) override
            {
                // Links cannot be 'activated', only main event queue processing is allowed
                assert(false);
                return -1;
            }

};

