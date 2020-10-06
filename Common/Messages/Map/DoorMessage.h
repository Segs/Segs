/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Components/BitStream.h"

#include <QtCore/QString>

enum DoorMessageStatus
{
    WAIT = 0,
    DONE = 1,
    DIALOG = 2,
};

namespace SEGSEvents
{
// [[ev_def:type]]
class DoorMessage final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    DoorMessageStatus m_delay_status;
    // [[ev_def:field]]
    QString m_msg;

    explicit DoorMessage() : GameCommandEvent(MapEventTypes::evDoorMessage) {}
    DoorMessage(DoorMessageStatus delay, QString msg) : GameCommandEvent(MapEventTypes::evDoorMessage),
        m_delay_status(delay),
        m_msg(msg)
    {
    }
    void    serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // packet 16

        bs.StorePackedBits(1, uint32_t(m_delay_status));
        bs.StoreString(m_msg);
    }
    EVENT_IMPL(DoorMessage)
};
} //end of SEGSEvents namespace
