/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "EventProcessor.h"
#include "SEGSTimer.h"

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
        Event *mb=new Timeout(current_time,timer_object->user_id(),this);
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
int EventProcessor::putq(Event *ev,ACE_Time_Value *timeout)
{
#ifdef EVENT_RECORDING
#endif
    return super::putq(ev,timeout);
}
//! @}
