/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup CommonServers Projects/CoX/Common/Servers
 * @{
 */

#include "MessageBus.h"
#include "MessageBusEndpoint.h"

#include "HandlerLocator.h"

#include "Components/SEGSTimer.h"
#include <cassert>

using namespace SEGSEvents;

namespace
{
    enum
    {
        Statistics_Timer = 1,
    };

    void removeEndpointFromUnorderedVector(std::vector<MessageBusEndpoint *> &vec,const MessageBusEndpoint *to_remove)
    {
        for(int i=0,total=vec.size(); i<total; ++i)
        {
            if(vec[i]==to_remove)
            {
                // swap - and - pop for very fast element removal
                std::swap(vec[i],vec.back());
                vec.pop_back();
                return;
            }
        }
    }
    ACE_Time_Value statistic_update_interval(0,1000*1000*15);
} // namespace

MessageBus::MessageBus()
{
    // world simulation ticks
    m_statistics_timer_id = addTimer(statistic_update_interval,false);
    startTimer(m_statistics_timer_id,&MessageBus::recalculateStatisitcs);
}

bool MessageBus::ReadConfigAndRestart()
{
    static bool is_running=false;
    //for now message bus has no configuration variables
    // later on it will be more flexible and need them.
    assert(!is_running);
    if(!is_running)
    {
        activate();
        is_running = true;
    }
    return true;
}

void MessageBus::subscribe(uint32_t type, MessageBusEndpoint *ep)
{
    if(type==ALL_EVENTS)
    {
        m_catch_all_subscribers.emplace_back(ep);
    }
    else
    {
        m_specific_subscriber_map[type].emplace_back(ep);
    }
}

void MessageBus::unsubscribe(uint32_t type, MessageBusEndpoint *e)
{
    if(type==ALL_EVENTS)
    {
        removeEndpointFromUnorderedVector(m_catch_all_subscribers,e);
    }
    else
    {
        auto iter = m_specific_subscriber_map.find(type);
        if(iter!=m_specific_subscriber_map.end())
            removeEndpointFromUnorderedVector(iter->second,e);
    }
}

void MessageBus::do_publish(Event *ev)
{
    assert(ev->get_ref_count()==1);
    for(MessageBusEndpoint *ep : m_catch_all_subscribers)
    {
        ep->putq(ev->shallow_copy());
    }
    auto iter = m_specific_subscriber_map.find(ev->type());
    if(iter!=m_specific_subscriber_map.end())
    {
        for(MessageBusEndpoint *ep : iter->second)
        {
            ep->putq(ev->shallow_copy());
        }
    }
}

void MessageBus::dispatch(Event *ev)
{
    do_publish(ev);
}

void MessageBus::recalculateStatisitcs()
{

}

void MessageBus::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void MessageBus::serialize_to(std::ostream &/*os*/)
{
    assert(false);
}

void postGlobalEvent(Event *ev)
{
    HandlerLocator::getMessageBus()->putq(ev);
}

///
/// \brief shutDownMessageBus will request that all message bus handling threads terminate
/// \note this cannot be called from one of the message bus handling threads.
void shutDownMessageBus()
{
    HandlerLocator::getMessageBus()->putq(Finish::s_instance->shallow_copy());
    HandlerLocator::getMessageBus()->wait();

}

//! @}
