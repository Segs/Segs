/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "GameCommandList.h"
#include "Logging.h"

#include "MapEvents.h"
#include "MapLink.h"

class TeamLooking final : public GameCommand
{
public:
    uint32_t m_num;         // size of array
    QString m_name;         // player name
    QString m_classname;    // player class
    QString m_origin;       // player origin
    uint32_t m_level;       // player level
    TeamLooking(QString &name, QString &classname, QString &origin, uint32_t level) : GameCommand(MapEventTypes::evTeamLooking),
        m_name(name),
        m_classname(classname),
        m_origin(origin),
        m_level(level)
    {
    }
    void    serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 25
        bs.StoreBits(32,m_num);
        bs.StoreString(m_name);
        bs.StoreString(m_classname);
        bs.StoreString(m_origin);
        bs.StoreBits(32,m_level);

        qCDebug(logTeams) << "Team Looking:" << m_num << m_name << m_classname << m_origin << m_level;
    }
    void    serializefrom(BitStream &src);
};
