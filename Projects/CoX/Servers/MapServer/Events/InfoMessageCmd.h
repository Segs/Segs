/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "GameCommandList.h"

#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>
enum class InfoType : uint8_t {
    COMBAT         = 1,
    DAMAGE         = 2,
    SVR_COM        = 3,
    NPC_SAYS       = 4,
    VILLAIN_SAYS   = 5,
    REGULAR        = 6,
    PRIVATE_COM    = 7,
    TEAM_COM       = 8,
    SUPERGROUP_COM = 9,
    NEARBY_COM     = 10,
    SHOUT_COM      = 11,
    REQUEST_COM    = 12,
    FRIEND_COM     = 13,
    ADMIN_COM      = 14,
    USER_ERROR     = 15,
    DEBUG_INFO     = 16,
    EMOTE          = 17,
    CHAT_TEXT      = 18,
    PROFILE_TEXT   = 19,
    HELP_TEXT      = 20,
    STD_TEXT       = 21,
};

class InfoMessageCmd : public GameCommand
{
public:
    QString     m_msg;
    InfoType    m_info_type;
                InfoMessageCmd(InfoType t,QString msg) : GameCommand(MapEventTypes::evInfoMessageCmd),
                    m_info_type(t),m_msg(msg)
                {
                }
        void    serializeto(BitStream &bs) const override {
                    bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient);
                    bs.StorePackedBits(2,uint8_t(m_info_type));
                    bs.StoreString(m_msg);
                }
        void    serializefrom(BitStream &src);
};
