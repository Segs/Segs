/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "MapEventTypes.h"
#include "GameCommand.h"
#include "Components/BitStream.h"

#include <QtCore/QString>

namespace SEGSEvents
{

// [[ev_def:type]]
class StandardDialogCmd final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    QString     m_msg;

    explicit StandardDialogCmd() : GameCommandEvent(evStandardDialogCmd) {}
    StandardDialogCmd(QString msg) : GameCommandEvent(evStandardDialogCmd),
        m_msg(msg)
    {
    }

    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 14
        bs.StoreString(m_msg);
    }
    EVENT_IMPL(StandardDialogCmd)
};

} // end of SEGSEvents namespace

