/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "EventProcessor.h"
#include <vector>

/**
 * @brief The MessageBusEndpoint class is responsible for:
 * * registering as a handler for various message types
 * * deregistering handlers from MessageBus on destruction,
 * * used by MessageBus to forward messages to underlying EventProcessor
 */
class MessageBusEndpoint
{
    EventProcessor &m_true_handler;
    MessageBusEndpoint(const MessageBusEndpoint &) = delete;
    MessageBusEndpoint &operator =(const MessageBusEndpoint &) = delete;
    std::vector<uint32_t> m_active_subscriptions;
public:
    MessageBusEndpoint(EventProcessor &h) : m_true_handler(h) {}
    ~MessageBusEndpoint();
    void putq(SEGSEvent *e)
    {
        m_true_handler.putq(e);
    }
    void subscribe(uint32_t ev_type);
    void unsubscribe(uint32_t ev_type);
};


