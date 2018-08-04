/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameDatabase Projects/CoX/Servers/GameDatabase
 * @{
 */

#include "GameDBSyncHandler.h"

#include "HandlerLocator.h"
#include "GameDBSyncEvents.h"
#include "MessageBus.h"

bool GameDBSyncHandler::per_thread_setup()
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    bool result = db_ctx.loadAndConfigure();
    if(!result)
        postGlobalEvent(new ServiceStatusMessage({"GameDbSync failed to load/configure",-1}));
    else
        postGlobalEvent(new ServiceStatusMessage({"GameDbSync loaded/configured",0}));
    return result;
}

void GameDBSyncHandler::dispatch(SEGSEvent *ev)
{
    // We are servicing a request from message queue, using dispatchSync as a common processing point.
    // nullptr result means that the given message is one-way
    switch (ev->type())
    {
    case GameDBEventTypes::evCharacterUpdate:
        on_character_update(static_cast<CharacterUpdateMessage *>(ev)); break;
    case GameDBEventTypes::evPlayerUpdate:
        on_player_update(static_cast<PlayerUpdateMessage *>(ev)); break;
    case GameDBEventTypes::evRemoveCharacterRequest:
        on_character_remove(static_cast<RemoveCharacterRequest *>(ev)); break;
    case GameDBEventTypes::evCostumeUpdate:
        on_costume_update(static_cast<CostumeUpdateMessage *>(ev)); break;
    case GameDBEventTypes::evGameAccountRequest:
        on_account_request(static_cast<GameAccountRequest *>(ev)); break;
    case GameDBEventTypes::evWouldNameDuplicateRequest:
        on_check_name_clash(static_cast<WouldNameDuplicateRequest *>(ev)); break;
    case GameDBEventTypes::evCreateNewCharacterRequest:
        on_create_new_char(static_cast<CreateNewCharacterRequest *>(ev)); break;
    case GameDBEventTypes::evGetEntityRequest:
        on_get_entity(static_cast<GetEntityRequest *>(ev)); break;
    case GameDBEventTypes::evGetPlayerFriendsRequest:
        on_get_player_friends(static_cast<GetPlayerFriendsRequest *>(ev)); break;
    default: assert(false); break;
    }
}

GameDBSyncHandler::GameDBSyncHandler(uint8_t id) : m_id(id)
{
    HandlerLocator::setGame_DB_Handler(id,this);
}

void GameDBSyncHandler::on_character_remove(RemoveCharacterRequest *msg)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());

    if (!db_ctx.removeCharacter(msg->m_data))
        msg->src()->putq(new GameDbErrorMessage({"on_character_remove : Game db error"}, msg->session_token()));
    else
        msg->src()->putq(new RemoveCharacterResponse({msg->m_data.slot_idx}, msg->session_token()));
}


void GameDBSyncHandler::on_character_update(CharacterUpdateMessage *msg)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    db_ctx.performUpdate(msg->m_data);
}

void GameDBSyncHandler::on_costume_update(CostumeUpdateMessage *msg)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    db_ctx.performUpdate(msg->m_data);
}

void GameDBSyncHandler::on_player_update(PlayerUpdateMessage *msg)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    db_ctx.performUpdate(msg->m_data);
}

void GameDBSyncHandler::on_client_options_update(SetClientOptionsMessage *msg)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    db_ctx.updateClientOptions(msg->m_data);
}

void GameDBSyncHandler::on_account_request(GameAccountRequest *msg)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    GameAccountResponseData resp;
    if(db_ctx.getAccount(msg->m_data,resp))
        msg->src()->putq(new GameAccountResponse(std::move(resp),msg->session_token()));
    else
        msg->src()->putq(new GameDbErrorMessage({"on_account_request:Game db error"},msg->session_token()));

}

void GameDBSyncHandler::on_check_name_clash(WouldNameDuplicateRequest * ev)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    WouldNameDuplicateResponseData resp;

    if(db_ctx.checkNameClash(ev->m_data,resp))
        ev->src()->putq(new WouldNameDuplicateResponse(std::move(resp),ev->session_token()));
    else
        ev->src()->putq(new GameDbErrorMessage({"on_check_name_clash:Game db error"},ev->session_token()));
}

void GameDBSyncHandler::on_create_new_char(CreateNewCharacterRequest * ev)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    CreateNewCharacterResponseData resp;

    if(db_ctx.createNewChar(ev->m_data,resp))
        ev->src()->putq(new CreateNewCharacterResponse(std::move(resp),ev->session_token()));
    else
        ev->src()->putq(new GameDbErrorMessage({"Game db error"},ev->session_token()));
}

void GameDBSyncHandler::on_get_entity(GetEntityRequest *ev)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    GetEntityResponseData resp;

    if(db_ctx.getEntity(ev->m_data,resp))
        ev->src()->putq(new GetEntityResponse(std::move(resp),ev->session_token()));
    else
        ev->src()->putq(new GameDbErrorMessage({"Game db error"},ev->session_token()));
}

void GameDBSyncHandler::on_get_player_friends(GetPlayerFriendsRequest *ev){
    GameDbSyncContext &db_ctx(m_db_context.localData());
    GetPlayerFriendsResponseData resp;

    //getPlayerFriends returns true if successful
    if(db_ctx.getPlayerFriends(ev->m_data,resp))
    {
        ev->src()->putq(new GetPlayerFriendsResponse(std::move(resp),ev->session_token()));
    }
    else
        ev->src()->putq(new GameDbErrorMessage({"Game db error"},ev->session_token()));
}
//! @}
