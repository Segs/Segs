/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"
#include "Servers/MapServer/EntityStorage.h"

// The ChatService handles chats, emotes...
class ChatService
{
private:
    EntityManager& ref_entity_mgr;

    QString process_random_boombox_emote();
    QString process_random_dice_emote();
    QString process_random_dance_emote();
    QString get_prepared_chat_message(const QString& name, const QString& msg_text);

    std::unique_ptr<SEGSEvents::ChatServiceToClientData> process_local_chat(Entity *sender, QString &msg_text);
    std::unique_ptr<SEGSEvents::ChatServiceToClientData> process_broadcast_chat(Entity *sender, QString &msg_text);
    std::unique_ptr<SEGSEvents::ChatServiceToClientData> process_request_chat(Entity *sender, QString &msg_text);
    std::unique_ptr<SEGSEvents::ChatServiceToClientData> process_private_chat(Entity *sender, QString &msg_text);
    std::unique_ptr<SEGSEvents::ChatServiceToClientData> process_team_chat(Entity *sender, QString &msg_text);
    std::unique_ptr<SEGSEvents::ChatServiceToClientData> process_supergroup_chat(Entity *sender, QString &msg_text);
    std::unique_ptr<SEGSEvents::ChatServiceToClientData> process_friends_chat(Entity *sender, QString &msg_text);
public:
    ChatService(EntityManager &em) : ref_entity_mgr(em) {};
    std::unique_ptr<SEGSEvents::ChatServiceToClientData> on_emote_command(Entity* ent, const QString& command);
    std::unique_ptr<SEGSEvents::ChatServiceToClientData> process_chat(Entity *sender, QString &msg_text);
    QString process_replacement_strings(Entity* ent, const QString &msg_text);
    bool isChatMessage(const QString& msg);
    bool has_emote_prefix(const QString &cmd);



protected:
};


