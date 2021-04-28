/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/EventProcessor.h"

#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>

#include <unordered_map>
#include <vector>

class MessageBusEndpoint;

class SEGSTimer;
/**
 * @brief The MessageBus class is responsible for forwarding incoming events to correct subscriber's endpoints
 *
 */

class MessageBus final : public EventProcessor
{
    std::unordered_map<uint32_t,std::vector<MessageBusEndpoint *> > m_specific_subscriber_map;
    std::vector<MessageBusEndpoint *> m_catch_all_subscribers;
    uint32_t m_statistics_timer_id;
    friend void postGlobalEvent(SEGSEvents::Event *ev);
    friend void shutDownMessageBus();
    friend class MessageBusEndpoint; // allow endpoints to register/unregister
public:

static constexpr uint32_t ALL_EVENTS = ~0U; // special event type allowing subscriptions to all incoming events

public:
                IMPL_ID(MessageBus)
                MessageBus();
                bool ReadConfigAndRestart();
private:
                ///
                /// \brief subscribe given endpoint \a ep to all events of \a type
                /// \param type specific event type, or ALL_EVENTS
                /// \param ep which endpoint to notify
                ///
        void    subscribe(uint32_t type, MessageBusEndpoint *ep);
                ///
                /// \brief unsubscribe from all events ( type == ALL_EVENTS )
                ///
        void    unsubscribe(uint32_t type,MessageBusEndpoint *);
                ///
                /// \brief do_publish will locate all handlers ( catch-all, and event specific ), and send message copies to them.
                /// \param ev this event will be shallow_copy'ied and putq'd to all subscribers
                ///
        void    do_publish(SEGSEvents::Event *ev);
        void    recalculateStatisitcs();
                // EventProcessor interface
protected:
        void    serialize_from(std::istream &is) override;
        void    serialize_to(std::ostream &os) override;
        void    dispatch(SEGSEvents::Event *ev) override;
};

void postGlobalEvent(SEGSEvents::Event *ev);
void shutDownMessageBus();
