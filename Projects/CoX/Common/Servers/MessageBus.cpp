/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup CommonServers Projects/CoX/Common/Servers
 * @{
 */

#include "MessageBus.h"
#include "MessageBusEndpoint.h"

#include "HandlerLocator.h"

#include "SEGSTimer.h"
#include <cassert>

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
    m_statistics_timer.reset(new SEGSTimer(this,(void *)Statistics_Timer,statistic_update_interval,false)); // world simulation ticks
}

bool MessageBus::ReadConfigAndRestart()
{
    static bool is_running=false;
    //for now message bus has no configuration variables
    // later on it will be more flexible and need them.
    assert(!is_running);
    if(!is_running) {
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

void MessageBus::do_publish(SEGSEvent *ev)
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

void MessageBus::dispatch(SEGSEvent *ev)
{
    if(ev->src()==this && ev->type()==SEGS_EventTypes::evTimeout)
    {
        recalculateStatisitcs();
        return;
    }
    do_publish(ev);
}

void MessageBus::recalculateStatisitcs()
{

}

void postGlobalEvent(SEGSEvent *ev)
{
    HandlerLocator::getMessageBus()->putq(ev);
}

///
/// \brief shutDownMessageBus will request that all message bus handling threads terminate
/// \note this cannot be called from one of the message bus handling threads.
void shutDownMessageBus()
{
    HandlerLocator::getMessageBus()->putq(SEGSEvent::s_ev_finish.shallow_copy());
    HandlerLocator::getMessageBus()->wait();

}

//! @}
