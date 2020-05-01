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
    SetKeybind* casted_ev = static_cast<SetKeybind *>(ev);

    auto func = [casted_ev](Entity* ent) -> void
    {
        KeyName key = static_cast<KeyName>(casted_ev->key);
        ModKeys mod = static_cast<ModKeys>(casted_ev->mods);

        ent->m_player->m_keybinds.setKeybind(casted_ev->profile, key, mod, casted_ev->command, casted_ev->is_secondary);
        qCDebug(logKeybinds) << "Setting keybind: " << casted_ev->profile << QString::number(casted_ev->key)
                              << QString::number(casted_ev->mods) << casted_ev->command << casted_ev->is_secondary;
    };

    return std::make_unique<ServiceToEntityData>(get_session_token(ev), func);
}

UPtrServiceToEntityData ClientOptionService::on_save_client_options(Event *ev)
{
    SaveClientOptions* casted_ev = static_cast<SaveClientOptions *>(ev);

    auto func = [casted_ev](Entity* ent) -> void
    {
        markEntityForDbStore(ent,DbStoreFlags::PlayerData);
        ent->m_player->m_options = casted_ev->data;
    };

    return std::make_unique<ServiceToEntityData>(get_session_token(ev), func);
}

UPtrServiceToEntityData ClientOptionService::on_remove_keybind(Event *ev)
{
    RemoveKeybind* casted_ev = static_cast<RemoveKeybind *>(ev);

    auto func = [casted_ev](Entity* ent) -> void
    {
        ent->m_player->m_keybinds.removeKeybind(casted_ev->profile,(KeyName &)casted_ev->key,(ModKeys &)casted_ev->mods);
        qCDebug(logKeybinds) << "Clearing Keybind: " << casted_ev->profile << QString::number(casted_ev->key) << QString::number(casted_ev->mods);
    };

    return std::make_unique<ServiceToEntityData>(get_session_token(ev), func);
}

UPtrServiceToEntityData ClientOptionService::on_select_keybind_profile(Event *ev)
{
    SelectKeybindProfile* casted_ev = static_cast<SelectKeybindProfile *>(ev);

    auto func = [casted_ev](Entity* ent) -> void
    {
        ent->m_player->m_keybinds.setKeybindProfile(casted_ev->profile);
        qCDebug(logKeybinds) << "Saving currently selected Keybind Profile. Profile name: " << casted_ev->profile;
    };

    return std::make_unique<ServiceToEntityData>(get_session_token(ev), func);
}

UPtrServiceToEntityData ClientOptionService::on_reset_keybinds(Event *ev)
{
    ResetKeybinds* casted_ev = static_cast<ResetKeybinds *>(ev);

    auto func = [casted_ev](Entity* ent) -> void
    {
        const GameDataStore &data(getGameData());
        const Parse_AllKeyProfiles &default_profiles(data.m_keybind_profiles);

        ent->m_player->m_keybinds.resetKeybinds(default_profiles);
        qCDebug(logKeybinds) << "Resetting Keybinds to defaults.";
    };

    return std::make_unique<ServiceToEntityData>(get_session_token(ev), func);
}

UPtrServiceToEntityData ClientOptionService::on_switch_viewpoint(Event *ev)
{
    SwitchViewPoint* casted_ev = static_cast<SwitchViewPoint *>(ev);

    auto func = [casted_ev](Entity* ent) -> void
    {
        ent->m_player->m_options.m_first_person_view = casted_ev->new_viewpoint_is_firstperson;
        qCDebug(logKeybinds) << "Saving viewpoint mode to ClientOptions" << casted_ev->new_viewpoint_is_firstperson;
    };

    return std::make_unique<ServiceToEntityData>(get_session_token(ev), func);
}
