/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"

struct MapClientSession;
class QString;

enum class MessageChannel : int;
namespace SEGSEvents
{

// [[ev_def:type]]
class InfoMessageCmd : public GameCommand
{
public:
    // [[ev_def:field]]
    QString         m_msg;
    // [[ev_def:field]]
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

} // end of SEGSEvents namespace


extern void sendInfoMessage(MessageChannel t, QString msg, MapClientSession *tgt);
