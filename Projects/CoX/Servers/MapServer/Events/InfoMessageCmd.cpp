/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "InfoMessageCmd.h"

#include "Servers/MapServer/DataHelpers.h"
#include "MapClientSession.h"
#include "Logging.h"

void InfoMessageCmd::serializeto(BitStream &bs) const
{
    bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient);
    bs.StorePackedBits(2,uint8_t(m_channel_type));
    bs.StoreString(m_msg);
}

void sendInfoMessage(MessageChannel t, QString msg, MapClientSession *tgt)
{

    InfoMessageCmd * res = new InfoMessageCmd(t,msg);
    res->m_target_player_id = getIdx(*tgt->m_ent);

    tgt->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(res));


    qCDebug(logInfoMsg).noquote() << "InfoMessage:"
             << "\n  Channel:" << int(res->m_channel_type)
             << "\n  Target:" << res->m_target_player_id
             << "\n  Message:" << res->m_msg;
}

//! @}
