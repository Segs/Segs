/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Components/SEGSEvent.h"
#include "Components/SEGSTimer.h"

#include <ace/Task_Ex_T.h>

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
using TimerStorage = std::unordered_map<uint32_t,std::unique_ptr<SEGSTimer>>;
protected:
        TimerStorage    m_registered_timers;
        uint32_t        m_next_timer_id=1;
private:
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
        //
        // Timer service methods
        //
        uint32_t        addTimer(const ACE_Time_Value &fire_delta_time, bool one_shot=false);
        uint32_t        startTimer(uint32_t id,std::function<void(const ACE_Time_Value &)> cb);
        /// \note using the following template function is possibly slightly less efficient then the above func
        template<class T>
        uint32_t        startTimer(uint32_t id,void (T::*cb)())
                        {
                            return startTimer(id,[this,cb](const ACE_Time_Value &) { (((T *)this)->*cb)(); });
                        }
        SEGSTimer &     accessTimer(uint32_t id);
        void            removeTimer(uint32_t id);
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
