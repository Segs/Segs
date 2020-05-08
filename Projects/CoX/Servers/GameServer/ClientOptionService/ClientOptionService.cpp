/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "Common/CRUDP_Protocol/ILink.h"
#include "Common/Servers/EventHelpers.h"
#include "ClientOptionService.h"
#include "GameData/Entity.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/map_definitions.h"
#include "GameData/EntityHelpers.h"
#include "Messages/Map/MapEvents.h"

UPtrServiceToEntityData ClientOptionService::on_set_keybind(Event *ev)
{
    auto func = [content_data = static_cast<SetKeybind*>(ev->shallow_copy())](Entity* ent) -> void
    {
        KeyName key = static_cast<KeyName>(content_data->key);
        ModKeys mod = static_cast<ModKeys>(content_data->mods);

        ent->m_player->m_keybinds.setKeybind(content_data->profile, key, mod, content_data->command, content_data->is_secondary);
        qCDebug(logKeybinds) << "Setting keybind: " << content_data->profile << QString::number(content_data->key)
                              << QString::number(content_data->mods) << content_data->command << content_data->is_secondary;

        content_data->release();
    };

    return std::make_unique<ServiceToEntityData>(get_session_token(ev), func);
}

UPtrServiceToEntityData ClientOptionService::on_save_client_options(Event *ev)
{
    auto func = [content_data = static_cast<SaveClientOptions*>(ev->shallow_copy())](Entity* ent) -> void
    {
        markEntityForDbStore(ent,DbStoreFlags::PlayerData);
        ent->m_player->m_options = content_data->data;

        content_data->release();
    };

    return std::make_unique<ServiceToEntityData>(get_session_token(ev), func);
}

UPtrServiceToEntityData ClientOptionService::on_remove_keybind(Event *ev)
{
    auto func = [content_data = static_cast<RemoveKeybind*>(ev->shallow_copy())](Entity* ent) -> void
    {
        ent->m_player->m_keybinds.removeKeybind(content_data->profile,(KeyName &)content_data->key,(ModKeys &)content_data->mods);
        qCDebug(logKeybinds) << "Clearing Keybind: " << content_data->profile << QString::number(content_data->key) << QString::number(content_data->mods);

        content_data->release();
    };

    return std::make_unique<ServiceToEntityData>(get_session_token(ev), func);
}

UPtrServiceToEntityData ClientOptionService::on_select_keybind_profile(Event *ev)
{
    auto func = [content_data = static_cast<SelectKeybindProfile*>(ev->shallow_copy())](Entity* ent) -> void
    {
        ent->m_player->m_keybinds.setKeybindProfile(content_data->profile);
        qCDebug(logKeybinds) << "Saving currently selected Keybind Profile. Profile name: " << content_data->profile;

        content_data->release();
    };

    return std::make_unique<ServiceToEntityData>(get_session_token(ev), func);
}

UPtrServiceToEntityData ClientOptionService::on_reset_keybinds(Event *ev)
{
    auto func = [content_data = static_cast<ResetKeybinds*>(ev->shallow_copy())](Entity* ent) -> void
    {
        const GameDataStore &data(getGameData());
        const Parse_AllKeyProfiles &default_profiles(data.m_keybind_profiles);

        ent->m_player->m_keybinds.resetKeybinds(default_profiles);
        qCDebug(logKeybinds) << "Resetting Keybinds to defaults.";

        content_data->release();
    };

    return std::make_unique<ServiceToEntityData>(get_session_token(ev), func);
}

UPtrServiceToEntityData ClientOptionService::on_switch_viewpoint(Event *ev)
{
    auto func = [content_data = static_cast<SwitchViewPoint *>(ev->shallow_copy())](Entity* ent) -> void
    {
        ent->m_player->m_options.m_first_person_view = content_data->new_viewpoint_is_firstperson;
        qCDebug(logKeybinds) << "Saving viewpoint mode to ClientOptions" << content_data->new_viewpoint_is_firstperson;

        content_data->release();
    };

    return std::make_unique<ServiceToEntityData>(get_session_token(ev), func);
}
