/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "TransactionService.h"
#include "GameData/Powers.h"
#include "GameData/Entity.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/Character.h"
#include "GameData/map_definitions.h"
#include "GameData/EntityHelpers.h"
#include "Messages/Map/MapEvents.h"
#include "Messages/Map/StoresEvents.h"
#include "MapServer/MapInstance.h"
#include "MapServer/MapClientSession.h"
#include "Servers/MapServer/DataHelpers.h"
#include <QtCore/QDebug>

using namespace SEGSEvents;

ServiceToClientData* TransactionService::on_store_sell_item(Entity* playerEnt, Event* ev)
{
    StoreSellItem* casted_ev = static_cast<StoreSellItem*>(ev);
    qCDebug(logStores) << "on_store_sell_item. NpcId: " << casted_ev->m_npc_idx << " isEnhancement: " << casted_ev->m_is_enhancement << " TrayNumber: " << casted_ev->m_tray_number << " enhancement_idx: " << casted_ev->m_enhancement_idx;
    Entity *e = getEntity(playerEnt->m_client, casted_ev->m_npc_idx);

    QString enhancement_name;
    CharacterEnhancement enhancement = playerEnt->m_char->m_char_data.m_enhancements[casted_ev->m_enhancement_idx];
    enhancement_name = enhancement.m_name + "_" + QString::number(enhancement.m_level);

    if(enhancement_name.isEmpty())
    {
        qCDebug(logStores) << "on_store_sell_item. EnhancementId " << casted_ev->m_enhancement_idx << " not found";
        return nullptr;
    }

    if(e->m_is_store && !e->m_store_items.empty())
    {
        //Find store in entity store list
        StoreTransactionResult result = Store::sellItem(e, enhancement_name);

        if(result.m_is_success)
        {
            modifyInf(*playerEnt->m_client, result.m_inf_amount);
            trashEnhancement(playerEnt->m_char->m_char_data, casted_ev->m_enhancement_idx);
            return new ServiceToClientData(playerEnt, result.m_message, MessageChannel::SERVER);
        }
        else
            qCDebug(logStores) << "Error processing sellItem";
    }
    else
        qCDebug(logStores) << "Entity is not a store or has no items";

    return nullptr;
}

ServiceToClientData* TransactionService::on_store_buy_item(Entity* playerEnt, Event* ev)
{
    StoreBuyItem* casted_ev = static_cast<StoreBuyItem *>(ev);
    qCDebug(logMapEvents) << "on_store_buy_item. NpcId: " << casted_ev->m_npc_idx << " ItemName: " << casted_ev->m_item_name;

    Entity *e = getEntity(playerEnt->m_client, casted_ev->m_npc_idx);
    StoreTransactionResult result = Store::buyItem(e, casted_ev->m_item_name);
    if(result.m_is_success)
    {
        modifyInf(*playerEnt->m_client, result.m_inf_amount);
        if(result.m_is_insp)
            giveInsp(*playerEnt->m_client, result.m_item_name);
        else
            giveEnhancement(*playerEnt->m_client, result.m_item_name, result.m_enhancement_lvl);

        // sendChatMessage(MessageChannel::SERVER,result.m_message,session.m_ent,session);
        return new ServiceToClientData(playerEnt, result.m_message, MessageChannel::SERVER);
    }
    else
        qCDebug(logStores) << "Error processing buyItem";

    return nullptr;
}

ServiceToClientData* TransactionService::on_trade_cancelled(Entity* ent, Event* /*ev*/)
{
    // unused event
    //TradeWasCancelledMessage* casted_ev = static_cast<TradeWasCancelledMessage *>(ev);

    if(ent->m_trade == nullptr)
    {
        // Trade already cancelled.
        // The client sends this many times while closing the trade window for some reason.
        return nullptr;
    }

    const uint32_t tgt_db_id = ent->m_trade->getOtherMember(*ent).m_db_id;
    Entity* tgt = getEntityByDBID(ent->m_client->m_current_map, tgt_db_id);
    if(tgt == nullptr)
    {
        // Only one side left in the game.
        discardTrade(*ent);

        const QString msg = "Trade cancelled because the other player left.";
        sendTradeCancel(*ent->m_client, msg);

        qCDebug(logTrades) << ent->name() << "cancelled a trade where target has disappeared";
        return nullptr;
    }

    discardTrade(*ent);
    discardTrade(*tgt);

    const QString msg_src = "You cancelled the trade with " + tgt->name() + ".";
    const QString msg_tgt = ent->name() + " canceled the trade.";
    sendTradeCancel(*ent->m_client, msg_src);
    sendTradeCancel(*tgt->m_client, msg_tgt);

    qCDebug(logTrades) << ent->name() << "cancelled a trade with" << tgt->name();

    // should be sending ServiceToClientData with commands from sendTradeCancel
    return nullptr;
}

ServiceToClientData* TransactionService::on_trade_updated(Entity* ent, Event* ev)
{
    TradeWasUpdatedMessage* casted_ev = static_cast<TradeWasUpdatedMessage *>(ev);

    Entity* tgt = getEntityByDBID(ent->m_client->m_current_map, casted_ev->m_info.m_db_id);

    if(tgt == nullptr)
        return nullptr;

    QString msg;
    TradeSystemMessages result = updateTrade(*ent, *tgt, casted_ev->m_info);

    switch(result)
    {
    case TradeSystemMessages::HAS_SENT_NO_TRADE:
        msg = "You have not sent a trade offer.";
        break;
    case TradeSystemMessages::TGT_RECV_NO_TRADE:
        msg = QString("%1 has not received a trade offer.").arg(tgt->name());
        break;
    case TradeSystemMessages::SRC_RECV_NO_TRADE:
        msg = QString("You are not considering a trade offer from %1.").arg(tgt->name());
        break;
    case TradeSystemMessages::SUCCESS:
    {
        // send tradeUpdate pkt to client
        Trade& trade = *ent->m_trade;
        TradeMember& trade_src = trade.getMember(*ent);
        TradeMember& trade_tgt = trade.getMember(*tgt);
        sendTradeUpdate(*ent->m_client, *tgt->m_client, trade_src, trade_tgt);

        if(ent->m_trade->isAccepted())
        {
            finishTrade(*ent, *tgt); // finish handling trade
            sendTradeSuccess(*ent->m_client, *tgt->m_client); // send tradeSuccess pkt to client
        }
        break;
    }
    default:
        msg = "Something went wrong with trade update!"; // this should never happen
    }

    return new ServiceToClientData(ent, msg, MessageChannel::SERVER);
}


