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

#include <cassert>
#include "Components/SEGSTimer.h"
#include "Components/EventProcessor.h"

/**
    \class SEGSTimer
    Instances of SEGSTimer class are used to schedule events in the future

    The simplest use of the SEGSTimers
    \code
    class YayService : public EventProcessor
    {
        void initializeAllTimers()
        {
            // in a construct/initialization routine of a class inheriting from EventProcessor
            m_timer_id = addTimer(interval_time);
            // at some later point we start the timer with a callback.
            startTimer(m_timer_id,&YayService::simpleCB);
        }
        void simpleCB() { printf("Yay!\n"); }
    );
    \endcode

*/

SEGSTimer::SEGSTimer(uint32_t user_id, const ACE_Time_Value &fire_delta_time, bool one_shot)
                    : m_user_id(user_id)
                    , m_fire_delta_time(fire_delta_time)
                    , m_one_shot(one_shot)
{
}

// TODO: Make sure there are no timeouts scheduled by this timer, maybe ACE takes care of that when timer is canceled ?
SEGSTimer::~SEGSTimer()
{
    cancel();
    m_target = nullptr;
    m_id = -1;
}

void SEGSTimer::start(CallbackFunction callback)
{
    assert(m_id==-1);
    m_callback = callback;
    if(m_one_shot)
        m_id = GlobalTimerQueue::instance()->schedule(m_target,this,ACE_OS::gettimeofday()+m_fire_delta_time);
    else
        m_id = GlobalTimerQueue::instance()->schedule(m_target,this,ACE_OS::gettimeofday()+m_fire_delta_time,m_fire_delta_time);
    assert(m_id!=-1);
}

void SEGSTimer::cancel()
{
    if(m_id!=-1)
        GlobalTimerQueue::instance()->cancel(m_id,nullptr);
    m_id=-1;
}

void SEGSTimer::reset()
{
    cancel();
    start(m_callback);
}

void SEGSTimer::reschedule( const ACE_Time_Value &new_time )
{
    cancel();
    m_fire_delta_time = new_time;
    start(m_callback);
}

//! @}
