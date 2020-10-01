/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameDatabase Projects/CoX/Servers/GameDatabase
 * @{
 */

#include "GameDBSyncHandler.h"

#include "HandlerLocator.h"
#include "Messages/GameDatabase/GameDBSyncEvents.h"
#include "MessageBus.h"

using namespace SEGSEvents;

bool GameDBSyncHandler::per_thread_startup()
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    bool result = db_ctx.loadAndConfigure();
    if(!result)
        postGlobalEvent(new ServiceStatusMessage({"GameDbSync failed to load/configure",-1},0));
    else
        postGlobalEvent(new ServiceStatusMessage({"GameDbSync loaded/configured",0},0));
    return result;
}

void GameDBSyncHandler::dispatch(Event *ev)
{
    // We are servicing a request from message queue, using dispatchSync as a common processing point.
    // nullptr result means that the given message is one-way
    switch (ev->type())
    {
    case GameDBEventTypes::evCharacterUpdateMessage:
        on_character_update(static_cast<CharacterUpdateMessage *>(ev)); break;
    case GameDBEventTypes::evPlayerUpdateMessage:
        on_player_update(static_cast<PlayerUpdateMessage *>(ev)); break;
    case GameDBEventTypes::evRemoveCharacterRequest:
        on_character_remove(static_cast<RemoveCharacterRequest *>(ev)); break;
    case GameDBEventTypes::evCostumeUpdateMessage:
        on_costume_update(static_cast<CostumeUpdateMessage *>(ev)); break;
    case GameDBEventTypes::evGameAccountRequest:
        on_account_request(static_cast<GameAccountRequest *>(ev)); break;
    case GameDBEventTypes::evWouldNameDuplicateRequest:
        on_check_name_clash(static_cast<WouldNameDuplicateRequest *>(ev)); break;
    case GameDBEventTypes::evCreateNewCharacterRequest:
        on_create_new_char(static_cast<CreateNewCharacterRequest *>(ev)); break;
    case GameDBEventTypes::evGetEntityRequest:
        on_get_entity(static_cast<GetEntityRequest *>(ev)); break;
    case GameDBEventTypes::evGetEntityByNameRequest:
        on_get_entity_by_name(static_cast<GetEntityByNameRequest *>(ev)); break;
    case GameDBEventTypes::evEmailCreateRequest:
        on_email_create(static_cast<EmailCreateRequest *>(ev)); break;
    case GameDBEventTypes::evEmailMarkAsReadMessage:
        on_email_mark_as_read(static_cast<EmailMarkAsReadMessage *>(ev)); break;
    case GameDBEventTypes::evEmailUpdateOnCharDeleteMessage:
        on_email_update_on_char_delete(static_cast<EmailUpdateOnCharDeleteMessage *>(ev)); break;
    case GameDBEventTypes::evEmailRemoveMessage:
        on_email_remove(static_cast<EmailRemoveMessage *>(ev)); break;
    case GameDBEventTypes::evGetEmailRequest:
        on_get_email(static_cast<GetEmailRequest *>(ev)); break;
    case GameDBEventTypes::evGetEmailsRequest:
        on_get_emails(static_cast<GetEmailsRequest *>(ev)); break;
    case GameDBEventTypes::evFillEmailRecipientIdRequest:
        on_fill_email_recipient_id(static_cast<FillEmailRecipientIdRequest *>(ev)); break;
    default: assert(false); break;
    }
}

void GameDBSyncHandler::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void GameDBSyncHandler::serialize_to(std::ostream &/*os*/)
{
    assert(false);
}
GameDBSyncHandler::GameDBSyncHandler(uint8_t id) : m_id(id)
{
    HandlerLocator::setGame_DB_Handler(id,this);
}

void GameDBSyncHandler::on_character_remove(RemoveCharacterRequest *msg)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());

    if(!db_ctx.removeCharacter(msg->m_data))
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

void GameDBSyncHandler::on_get_entity_by_name(GetEntityByNameRequest *ev)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    GetEntityByNameResponseData resp;

    if(db_ctx.getEntityByName(ev->m_data, resp))
        ev->src()->putq(new GetEntityByNameResponse(std::move(resp), ev->session_token()));
    else
        ev->src()->putq(new GameDbErrorMessage({"Game db error"},ev->session_token()));
}

void GameDBSyncHandler::on_email_create(EmailCreateRequest* ev)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    EmailCreateResponseData resp;

    if(db_ctx.createEmail(ev->m_data, resp))
        HandlerLocator::getEmail_Handler()->putq(new EmailCreateResponse(std::move(resp), ev->session_token()));
    else
        HandlerLocator::getEmail_Handler()->putq(new GameDbErrorMessage({"Game db error"},ev->session_token()));
}

void GameDBSyncHandler::on_email_mark_as_read(EmailMarkAsReadMessage* msg)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    db_ctx.markEmailAsRead(msg->m_data);
}

void GameDBSyncHandler::on_email_update_on_char_delete(EmailUpdateOnCharDeleteMessage* msg)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    db_ctx.updateEmailOnCharDelete(msg->m_data);
}

void GameDBSyncHandler::on_email_remove(EmailRemoveMessage* msg)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    db_ctx.deleteEmail(msg->m_data);
}

void GameDBSyncHandler::on_get_email(GetEmailRequest* ev)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    GetEmailResponseData resp;

    if(db_ctx.getEmail(ev->m_data, resp))
        HandlerLocator::getEmail_Handler()->putq(new GetEmailResponse(std::move(resp),ev->session_token()));
    else
        HandlerLocator::getEmail_Handler()->putq(new GameDbErrorMessage({"Game db error"},ev->session_token()));
}

void GameDBSyncHandler::on_get_emails(GetEmailsRequest* ev)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    GetEmailsResponseData resp;

    // TODO: Investigate why I can't use ev->src()
    if(db_ctx.getEmails(ev->m_data, resp))
        HandlerLocator::getEmail_Handler()->putq(new GetEmailsResponse(std::move(resp), ev->session_token()));
    else
        ev->src()->putq(new GameDbErrorMessage({"Game db error"},ev->session_token()));
}

void GameDBSyncHandler::on_fill_email_recipient_id(FillEmailRecipientIdRequest *ev)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    FillEmailRecipientIdResponseData resp;

    if(db_ctx.fillEmailRecipientId(ev->m_data, resp))
        HandlerLocator::getEmail_Handler()->putq(new FillEmailRecipientIdResponse(std::move(resp), ev->session_token()));
    else
        HandlerLocator::getEmail_Handler()->putq(new FillEmailRecipientIdErrorMessage(
            {ev->m_data.m_sender_id, ev->m_data.m_recipient_name}, ev->session_token()));

}

//! @}
