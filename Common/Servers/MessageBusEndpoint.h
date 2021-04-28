/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/EventProcessor.h"
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
    void putq(SEGSEvents::Event *e)
    {
        m_true_handler.putq(e);
    }
    void subscribe(uint32_t ev_type);
    void unsubscribe(uint32_t ev_type);
};


