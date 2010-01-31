#pragma once
#include <ace/Timer_Queue_Adapters.h>
#include <ace/Timer_Heap.h>
#include <ace/Timer_List.h>
#include <ace/Timer_Hash.h>
#include <ace/Timer_Wheel.h>
#include <ace/Activation_Queue.h>

class EventProcessor;
//! Instances of SEGSTimer class are used to schedule events in the future
class SEGSTimer
{
protected:
    int                 m_id;
    void *              m_data;
    EventProcessor *    m_target;
    ACE_Time_Value      m_fire_delta_time;
public:
                        SEGSTimer(EventProcessor *m_processor,void *data,const ACE_Time_Value &fire_delta_time);
                        ~SEGSTimer();
    void                schedule();
    void                cancel();
    void                reset();
    void                reschedule(const ACE_Time_Value &new_time);
    EventProcessor *    target() const {return m_target;}
    void *              data() const {return m_data;}
};
