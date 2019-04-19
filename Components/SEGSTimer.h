/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <ace/Timer_Queue_Adapters.h>
//#include <ace/Timer_Heap.h>
#include <ace/Timer_List.h>
#include <ace/Timer_Hash.h>
//#include <ace/Timer_Wheel.h>
#include <ace/Activation_Queue.h>

class EventProcessor;
//! Instances of SEGSTimer class are used to schedule events in the future
class SEGSTimer
{
protected:
    long                m_id;
    uint64_t            m_user_id;
    EventProcessor *    m_target;
    ACE_Time_Value      m_fire_delta_time;
    bool                m_one_shot;
public:
                        SEGSTimer(EventProcessor *m_processor,uint64_t data,const ACE_Time_Value &fire_delta_time,bool one_shot=true);
                        ~SEGSTimer();
    void                schedule();
    void                cancel();
    void                reset();
    void                reschedule(const ACE_Time_Value &new_time);
    EventProcessor *    target() const {return m_target;}
    uint64_t            user_id() const {return m_user_id;}
    void                schedule_repeatable(); 
};
typedef ACE_Thread_Timer_Queue_Adapter<ACE_Timer_List> Thread_Timer_Queue;
typedef ACE_Singleton<Thread_Timer_Queue,ACE_Thread_Mutex> GlobalTimerQueue;
