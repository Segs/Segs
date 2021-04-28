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

namespace SEGSEvents
{

// [[ev_def:type]]
class EmailRead final : public GameCommandEvent
{
public:
    explicit EmailRead() : GameCommandEvent(MapEventTypes::evEmailRead) {}
    EmailRead(const int id, const QString &message, const QString sender) : GameCommandEvent(MapEventTypes::evEmailRead),
        m_id(id), m_message(message), m_sender_name(sender)
    {
    }

    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, type()-MapEventTypes::evFirstServerToClient);
        bs.StoreBits(32, m_id);
        bs.StoreString(m_message);
        bs.StorePackedBits(1, m_count);
        bs.StoreString(m_sender_name);
    }

    // [[ev_def:field]]
    int m_id;
    // [[ev_def:field]]
    QString m_message;
    // [[ev_def:field]]
    int m_count = 1; //Doesn't do anything in Issue 0, seemingly, so hardcoding as 1
    // [[ev_def:field]]
    QString m_sender_name;

    EVENT_IMPL(EmailRead)
};

} // end of SEGSEvents namespace

