#include <cassert>
#include "EventProcessor.h"
#include "SEGSTimer.h"
int EventProcessor::open( void *args /* = 0 */ )
{
        return super::open(args);
}

int EventProcessor::handle_timeout( const ACE_Time_Value &current_time, const void *act /* = 0 */ )
{
    const SEGSTimer *timer_object = static_cast<const SEGSTimer *>(act);
    assert(timer_object!=0);
    // if target is known
    if(timer_object->target())
    {
        SEGSEvent *mb=new TimerEvent(current_time,timer_object->data());
        // post a new event to it
        return timer_object->target()->putq(mb);
    }
        return 0;
}

int EventProcessor::svc( void )
{
        SEGSEvent *mb;
        //ACE_Time_Value prev_val(ACE_OS::gettimeofday());
        while(getq(mb,0)!=-1)
        {
                if(mb->type()==0)
                {
                        mb->release();
                        return 0;
                }
                dispatch(mb);
                mb->release();
        }
        return 0;
}
