/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include "GameDBSyncContext.h"
#include "EventProcessor.h"

#include <QThreadStorage>

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

class GameDBSyncHandler final : public EventProcessor
{
    /// Qt requires each db connection to be created and used in same thread
    /// in case there are more `activated` handlers
    QThreadStorage<GameDbSyncContext> m_db_context;
    // EventProcessor interface
    bool per_thread_setup() override;
    void dispatch(SEGSEvent *ev) override;
    void on_character_update(CharacterUpdateMessage *msg);
    void on_costume_update(CostumeUpdateMessage *msg);
    void on_gui_update(GuiUpdateMessage *msg);
    void on_options_update(OptionsUpdateMessage* msg);
    void on_keybinds_update(KeybindsUpdateMessage* msg);
    void on_player_update(PlayerUpdateMessage* msg);
    void on_client_options_update(SetClientOptionsMessage* msg);
    void on_account_request(GameAccountRequest *msg);
    void on_character_remove(RemoveCharacterRequest *msg);
    void on_check_name_clash(WouldNameDuplicateRequest *ev);
    void on_create_new_char(CreateNewCharacterRequest *ev);
    void on_get_entity(GetEntityRequest *ev);
    // This is an unique ID that links this DB with it's Game Server
    uint8_t m_id;
public:
    GameDBSyncHandler(uint8_t id);
};
