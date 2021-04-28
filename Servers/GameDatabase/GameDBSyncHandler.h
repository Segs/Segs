/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameDBSyncContext.h"
#include "Components/EventProcessor.h"

#include <QThreadStorage>

namespace SEGSEvents
{
struct CharacterUpdateMessage;
struct CostumeUpdateMessage;
struct GuiUpdateMessage;
struct OptionsUpdateMessage;
struct KeybindsUpdateMessage;
struct SetClientOptionsMessage;
struct PlayerUpdateMessage;
struct GameAccountRequest;
struct RemoveCharacterRequest;
struct WouldNameDuplicateRequest;
struct CreateNewCharacterRequest;
struct GetEntityRequest;
struct GetEntityByNameRequest;
struct EmailCreateRequest;
struct EmailCreateResponse;
struct EmailMarkAsReadMessage;
struct EmailUpdateOnCharDeleteMessage;
struct EmailRemoveMessage;
struct GetEmailRequest;
struct GetEmailsRequest;
struct FillEmailRecipientIdRequest;
}

class GameDBSyncHandler final : public EventProcessor
{
    /// Qt requires each db connection to be created and used in same thread
    /// in case there are more `activated` handlers
    QThreadStorage<GameDbSyncContext> m_db_context;
    // EventProcessor interface
protected:
    bool per_thread_startup() override;
    void dispatch(SEGSEvents::Event*ev) override;
    void serialize_from(std::istream &is) override;
    void serialize_to(std::ostream &is) override;
    void on_character_update(SEGSEvents::CharacterUpdateMessage *msg);
    void on_costume_update(SEGSEvents::CostumeUpdateMessage *msg);
    void on_player_update(SEGSEvents::PlayerUpdateMessage* msg);
    void on_client_options_update(SEGSEvents::SetClientOptionsMessage* msg);
    void on_account_request(SEGSEvents::GameAccountRequest *msg);
    void on_character_remove(SEGSEvents::RemoveCharacterRequest *msg);
    void on_check_name_clash(SEGSEvents::WouldNameDuplicateRequest *ev);
    void on_create_new_char(SEGSEvents::CreateNewCharacterRequest *ev);
    void on_get_entity(SEGSEvents::GetEntityRequest *ev);
    void on_get_entity_by_name(SEGSEvents::GetEntityByNameRequest *ev);

    // email stuff
    void on_email_create(SEGSEvents::EmailCreateRequest *ev);
    void on_email_mark_as_read(SEGSEvents::EmailMarkAsReadMessage *msg);
    void on_email_update_on_char_delete(SEGSEvents::EmailUpdateOnCharDeleteMessage *msg);
    void on_email_remove(SEGSEvents::EmailRemoveMessage *msg);
    void on_get_email(SEGSEvents::GetEmailRequest* ev);
    void on_get_emails(SEGSEvents::GetEmailsRequest *ev);
    void on_fill_email_recipient_id(SEGSEvents::FillEmailRecipientIdRequest *ev);

    // This is an unique ID that links this DB with it's Game Server
    uint8_t m_id;
public:
    IMPL_ID(GameDBSyncHandler)
    GameDBSyncHandler(uint8_t id);
};
