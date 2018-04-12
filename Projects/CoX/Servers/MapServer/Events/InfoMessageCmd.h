/*
 * Super Entity Game Server Project
 * http://github.com/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "GameCommandList.h"


struct MapClientSession;
class QString;

enum class MessageChannel : int;

class InfoMessageCmd : public GameCommand
{
public:
    QString         m_msg;
    MessageChannel  m_channel_type;
    int             m_target_player_id;
    virtual         ~InfoMessageCmd() = default;
                    InfoMessageCmd(MessageChannel t, const QString &msg) : GameCommand(MapEventTypes::evInfoMessageCmd),
                        m_msg(msg),m_channel_type(t)
                    {
                    }
    void            serializeto(BitStream &bs) const override;
    void            serializefrom(BitStream &src);
};

extern void sendInfoMessage(MessageChannel t, QString msg, MapClientSession *tgt);
