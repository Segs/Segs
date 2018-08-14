/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"

#include "MapEvents.h"

#include <QtCore/QString>

namespace SEGSEvents
{
// [[ev_def:type]]
class StandardDialogCmd final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    QString     m_msg;
                StandardDialogCmd() : GameCommandEvent(MapEventTypes::evStandardDialogCmd) {}
                StandardDialogCmd(QString msg) : GameCommandEvent(MapEventTypes::evStandardDialogCmd),m_msg(msg)
                {
                }

        void    serializeto(BitStream &bs) const override {
                    bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient);
                    bs.StoreString(m_msg);
                }
        void    serializefrom(BitStream &src);
        EVENT_IMPL(StandardDialogCmd)
};
} // end of SEGSEvents namespace

