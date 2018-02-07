#define DEBUG_CHAT
//#include "InfoMessageCmd.h"
#include "Entity.h"
#include "MapClient.h"
#include "Servers/MapServer/DataHelpers.h"

void InfoMessageCmd::serializeto(BitStream &bs) const
{
    bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient);
    bs.StorePackedBits(2,uint8_t(m_channel_type));
    bs.StoreString(m_msg);
}

void sendInfoMessage(MessageChannel t, QString msg, MapClient *tgt)
{

    InfoMessageCmd * res = new InfoMessageCmd(t,msg);
    res->m_target_player_id = getIdx(*tgt->char_entity());

    tgt->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(res));

#ifdef DEBUG_CHAT
    qDebug().noquote() << "InfoMessage:"
             << "\n  Channel:" << int(res->m_channel_type)
             << "\n  Target:" << res->m_target_player_id
             << "\n  Message:" << res->m_msg;
#endif
}
