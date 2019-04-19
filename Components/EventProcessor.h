/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "SEGSEvent.h"
#include <ace/Task_Ex_T.h>
#include <iosfwd>
// Independent Task, each EventProcessor when activated dequeues SEGSEvents,
// and dispatches them
class EventSrc : public ACE_Task_Ex<ACE_MT_SYNCH,SEGSEvents::Event>
{
public:
    using ACE_Task_Ex<ACE_MT_SYNCH,SEGSEvents::Event>::ACE_Task_Ex;
};
/** Independent Task, each EventProcessor when activated dequeues SEGSEvents,
    and dispatches them
*/
class EventProcessor : public EventSrc
{
using super = EventSrc;


        int             svc() final;
public:
        int             handle_timeout(const ACE_Time_Value &current_time, const void *act /* = 0 */) override final;
        int             open(void *args = nullptr) override;
        int             process_single_event();
virtual int             putq(SEGSEvents::Event *ev, ACE_Time_Value *timeout=nullptr);
virtual uint64_t        get_id() const = 0;
protected:
                        /// Called in svc before it starts servicing events, if it returns false, the svc will return -1
                        /// thus ending that particular thread
virtual bool            per_thread_startup() { return true; }
virtual void            per_thread_shutdown() { ; }

virtual void            dispatch(SEGSEvents::Event *ev)=0;
virtual void            serialize_from(std::istream &is) = 0;
virtual void            serialize_to(std::ostream &is) = 0;
};
#define IMPL_ID(EventProcessorChildType)\
    enum : uint64_t { processor_id = CompileTimeUtils::hash_64_fnv1a_const(#EventProcessorChildType) };\
    uint64_t        get_id() const override { return processor_id; }

inline void shutdown_event_processor_and_wait(EventProcessor *ep)
{
    if(ep && ep->thr_count())
    {
        ep->putq(SEGSEvents::Finish::s_instance->shallow_copy());
        ep->wait();
    }
}
