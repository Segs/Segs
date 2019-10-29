/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <ace/Timer_Queue_Adapters.h>
//#include <ace/Timer_Heap.h>
//#include <ace/Timer_Wheel.h>
#include <ace/Timer_List.h>
#include <ace/Timer_Hash.h>
#include <ace/Activation_Queue.h>

#include <functional>
#include <cassert>

class EventProcessor;
class SEGSTimer
{
    using CallbackFunction = std::function<void(const ACE_Time_Value &)>;
    friend EventProcessor; // allow EventProcessor to set itself as our target when this timer is added to it.
protected:
    long                m_id = -1;
    uint32_t            m_user_id;
    EventProcessor *    m_target = nullptr;
    ACE_Time_Value      m_fire_delta_time;
    bool                m_one_shot;
    CallbackFunction    m_callback;
public:
                        SEGSTimer(uint32_t user_id,const ACE_Time_Value &fire_delta_time,bool one_shot=true);
                        ~SEGSTimer();
    void                start(CallbackFunction callback);
    void                cancel();
    void                reset();
    void                reschedule(const ACE_Time_Value &new_time);
    EventProcessor *    target() const {return m_target;}
    uint32_t            id() const {return m_user_id;}
    void                callback(const ACE_Time_Value &v) { assert(m_callback); m_callback(v); }
protected:
    void                setTarget(EventProcessor *t) {
                            assert(m_id==-1); // we were not scheduled yet.
                            m_target = t;
                        }
};
typedef ACE_Thread_Timer_Queue_Adapter<ACE_Timer_List> Thread_Timer_Queue;
typedef ACE_Singleton<Thread_Timer_Queue,ACE_Thread_Mutex> GlobalTimerQueue;
