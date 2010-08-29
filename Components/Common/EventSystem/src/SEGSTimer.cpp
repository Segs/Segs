#include "SEGSTimer.h"
#include "EventProcessor.h"

SEGSTimer::SEGSTimer( EventProcessor *m_processor,void *data,const ACE_Time_Value &fire_delta_time, bool one_shot) 
                    : 
                    m_id(-1),m_data(data),m_target(m_processor),m_fire_delta_time(fire_delta_time),m_one_shot(one_shot)
{
    if(m_one_shot)
        schedule();
    else
        schedule_repeatable();

}
// TODO: Make sure there are no timeouts scheduled by this timer, maybe ACE takes care of that when timer is canceled ?
SEGSTimer::~SEGSTimer()
{
    cancel();
}

void SEGSTimer::schedule()
{
    ACE_ASSERT(m_id==-1);
    m_id = GlobalTimerQueue::instance()->schedule(m_target,this,ACE_OS::gettimeofday()+m_fire_delta_time);
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
    if(m_one_shot)
        schedule();
    else
        schedule_repeatable();
}

void SEGSTimer::reschedule( const ACE_Time_Value &new_time )
{
    cancel();
    m_fire_delta_time = new_time;
    schedule();
}

void SEGSTimer::schedule_repeatable()
{
    ACE_ASSERT(m_id==-1);
    m_id = GlobalTimerQueue::instance()->schedule(m_target,this,ACE_OS::gettimeofday()+m_fire_delta_time,m_fire_delta_time);
}