#include "SEGSTimer.h"
#include "EventProcessor.h"
typedef ACE_Thread_Timer_Queue_Adapter<ACE_Timer_List> Thread_Timer_Queue;
typedef ACE_Singleton<Thread_Timer_Queue,ACE_Thread_Mutex> GlobalTimerQueue;

SEGSTimer::SEGSTimer( EventProcessor *m_processor,void *data,const ACE_Time_Value &fire_delta_time ) 
                    : 
                    m_id(-1),m_target(m_processor),m_fire_delta_time(fire_delta_time)
{
    schedule();
}

SEGSTimer::~SEGSTimer()
{
    cancel();
}

void SEGSTimer::schedule()
{
    ACE_ASSERT(m_id==-1);
    m_id = GlobalTimerQueue::instance()->schedule(m_target,m_data,ACE_OS::gettimeofday()+m_fire_delta_time);
}

void SEGSTimer::cancel()
{
    if(m_id!=-1)
        GlobalTimerQueue::instance()->cancel(m_id,0);
    m_id=-1;
}

void SEGSTimer::reset()
{
    cancel();
    schedule();
}

void SEGSTimer::reschedule( const ACE_Time_Value &new_time )
{
    cancel();
    m_fire_delta_time = new_time;
    schedule();
}
