/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "GameCommandList.h"

#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>

class StandardDialogCmd final : public GameCommand
{
public:
    QString     m_msg;
                StandardDialogCmd(QString msg) : GameCommand(MapEventTypes::evStandardDialogCmd),m_msg(msg)
                {
                }

        void    serializeto(BitStream &bs) const override {
                    bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient);
                    bs.StoreString(m_msg);
                }
        void    serializefrom(BitStream &src);
};
