/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "EventProcessor.h"
#include "SEGSTimer.h"

#include <cassert>
#include <iso646.h> // visual studio needs this

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
        SEGSEvent *mb=new TimerEvent(current_time,timer_object->data(),this);
        // post a new event to it
        return timer_object->target()->putq(mb);
    }
    return 0;
}

int EventProcessor::svc( )
{
    if(not this->per_thread_setup())
        return -1;
    SEGSEvent *mb;

    while(getq(mb,nullptr)!=-1)
    {
        if(mb->type()==SEGS_EventTypes::evFinish)
        {
            if (thr_count() > 1)
                putq(mb); // put this back on our message queue, our siblings will receive it and shut down as well
            else
            {
                mb->release();
                this->flush();
            }
            return 0;
        }
        dispatch(mb);
        mb->release();
    }
    return 0;
}

//! @}
