/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "Components/EventProcessor.h"
#include "Components/SEGSTimer.h"
#include "Components/TimeEvent.h"

#include <cassert>
#ifdef _MSC_VER
#include <iso646.h> // visual studio needs this
#endif

using namespace SEGSEvents;

int EventProcessor::open( void *args /* = 0 */ )
{
    return super::open(args);
}

int EventProcessor::handle_timeout( const ACE_Time_Value &current_time, const void *act /* = 0 */ )
{
    const SEGSTimer *timer_object = static_cast<const SEGSTimer *>(act);
    assert(timer_object!=nullptr);
    // if target is known
    if(timer_object->target())
    {
        Event *mb=new Timeout(current_time,timer_object->id(),this);
        // post a new event to it
        return timer_object->target()->putq(mb);
    }
    return 0;
}

int EventProcessor::svc( )
{
    if(not this->per_thread_startup())
        return -1;
    Event *mb;

    while(getq(mb,nullptr)!=-1)
    {
        if(mb->type()==evFinish)
        {
            if(thr_count() > 1)
                putq(mb); // put this back on our message queue, our siblings will receive it and shut down as well
            else
            {
                mb->release();
                this->flush();
            }
            return 0;
        }

        if(mb->type()==evTimeout)
        {
            Timeout *to = static_cast<Timeout *>(mb);
            auto iter = m_registered_timers.find(to->timer_id());
            assert(iter!=m_registered_timers.end());
            iter->second->callback(to->arrival_time());
        }
        else
            dispatch(mb);
        mb->release();
    }
    this->per_thread_shutdown();
    return 0;
}
/**
 * @brief This is mostly used by unit testing and debugging functions.
 * @return 0 if no more events on queue, -1 if finish event was encountered., 1 if everything went ok.
 */
int EventProcessor::process_single_event()
{
    // this event processor cannot have any threads running to allow single-stepping
    assert(thr_count() < 1);
    Event *mb;
    ACE_Time_Value tv = ACE_OS::gettimeofday();
    if(getq(mb,&tv)==-1)
        return 0;
    if(mb->type()==SEGSEvents::evFinish)
    {
        return -1;
    }
    dispatch(mb);
    mb->release();
    return 1;
}
int EventProcessor::putq(Event *ev,ACE_Time_Value *timeout)
{
#ifdef EVENT_RECORDING
#endif
    return super::putq(ev,timeout);
}
/**
 * @brief Add a new timer to this EventProcessor.
 * @param fire_delta_time - time after which the timer will trigger, if @arg one_shot is false this is the interval
 * @param one_shot - when set, the timer will only fire once.
 * @return the id of created timer
 * @note created one-shot timers are *NOT* removed after the callback returns.
 */
uint32_t EventProcessor::addTimer(const ACE_Time_Value &fire_delta_time,bool one_shot)
{
    uint32_t tmr_id = m_next_timer_id++;
    m_registered_timers.emplace(tmr_id,std::make_unique<SEGSTimer>(tmr_id,fire_delta_time,one_shot));
    return tmr_id;
}
/**
 * @brief Starts the given timer which will call the provided function
 * @param id is an identifier of a previously created timer
 * @param cb is a std::function that holds a callback that will be called
 * @return the id of a timer
 * The returned value can be used to write 'compressed' code :
 * @code
 * m_timer_id = startTimer(addTimer(timer_period),&ThisClass::CBFunc);
 */
uint32_t EventProcessor::startTimer(uint32_t id, std::function<void (const ACE_Time_Value &)> cb)
{
    assert(m_registered_timers.find(id)!=m_registered_timers.end());
    SEGSTimer &tmr(*m_registered_timers[id]);
    tmr.setTarget(this);
    tmr.start(cb);
    return id;
}
/**
 * @brief This method allows access to a timer struct (allows changing period/stopping/restarting etc)
 * @param id is assumed to be a valid timer id.
 * @return reference to a SEGSTimer object
 */
SEGSTimer &EventProcessor::accessTimer(uint32_t id)
{
    assert(m_registered_timers.find(id)!=m_registered_timers.end());
    return *m_registered_timers[id];
}
/**
 * @brief Remove/destroy the timer that corresponds to provided @arg id
 * @param id is assumed to be a valid timer id.
 */
void EventProcessor::removeTimer(uint32_t id)
{
    assert(m_registered_timers.find(id)!=m_registered_timers.end());
    m_registered_timers.erase(id);
}

//! @}
