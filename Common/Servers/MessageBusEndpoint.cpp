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

#include "MessageBusEndpoint.h"

#include "HandlerLocator.h"
#include "MessageBus.h"

MessageBusEndpoint::~MessageBusEndpoint()
{
    for(uint32_t ev_type : m_active_subscriptions)
        HandlerLocator::getMessageBus()->unsubscribe(ev_type,this);
}

void MessageBusEndpoint::subscribe(uint32_t ev_type)
{
    HandlerLocator::getMessageBus()->subscribe(ev_type,this);
    m_active_subscriptions.emplace_back(ev_type);
}

void MessageBusEndpoint::unsubscribe(uint32_t ev_type)
{
    HandlerLocator::getMessageBus()->unsubscribe(ev_type,this);
    for(size_t i=0,total=m_active_subscriptions.size(); i<total; ++i)
    {
        if(m_active_subscriptions[i]==ev_type)
        {
            std::swap(m_active_subscriptions[i],m_active_subscriptions.back());
            m_active_subscriptions.pop_back();
            break;
        }
    }
}

//! @}
