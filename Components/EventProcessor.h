/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <ace/Task_Ex_T.h>
#include "SEGSEvent.h"
// Independent Task, each EventProcessor when activated dequeues SEGSEvents,
// and dispatches them
class EventProcessor : public ACE_Task_Ex<ACE_MT_SYNCH,SEGSEvent>
{
typedef ACE_Task_Ex<ACE_MT_SYNCH,SEGSEvent> super;


        int             svc(void) final;
public:
        int             open(void *args = nullptr);
        int             handle_timeout(const ACE_Time_Value &current_time, const void *act /* = 0 */);
                        /// Called in svc before start of event servicing, if it returns false, the svc will return -1
                        /// thus ending that particular thread
virtual bool            per_thread_setup() { return true; }
virtual void            dispatch(SEGSEvent *ev)=0;
};
