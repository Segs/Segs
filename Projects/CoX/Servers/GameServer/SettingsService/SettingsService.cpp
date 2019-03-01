/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "SettingsService.h"
#include "GameData/Entity.h"
#include "GameData/playerdata_definitions.h"
#include "Common/Servers/InternalEvents.h"
#include "Common/Servers/ClientManager.h"
#include "Servers/MapServer/MapClientSession.h"
#include "Servers/MapServer/MapInstance.h"
#include "GameData/map_definitions.h"
#include "Messages/Map/MapEvents.h"
#include <QtCore/QDebug>

using namespace SEGSEvents;


void SettingsService::on_set_keybind(SetKeybind *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    KeyName key = static_cast<KeyName>(ev->key);
    ModKeys mod = static_cast<ModKeys>(ev->mods);

    ent->m_player->m_keybinds.setKeybind(ev->profile, key, mod, ev->command, ev->is_secondary);
    //qCDebug(logMapEvents) << "Setting keybind: " << ev->profile << QString::number(ev->key) << QString::number(ev->mods) << ev->command << ev->is_secondary;
}


void SettingsService::on_remove_keybind(RemoveKeybind *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_keybinds.removeKeybind(ev->profile,(KeyName &)ev->key,(ModKeys &)ev->mods);
    //qCDebug(logMapEvents) << "Clearing Keybind: " << ev->profile << QString::number(ev->key) << QString::number(ev->mods);
}

void SettingsService::on_reset_keybinds(ResetKeybinds *ev)
{
    const GameDataStore &data(getGameData());
    const Parse_AllKeyProfiles &default_profiles(data.m_keybind_profiles);

    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_keybinds.resetKeybinds(default_profiles);
    //qCDebug(logMapEvents) << "Resetting Keybinds to defaults.";
}

void SettingsService::on_select_keybind_profile(SelectKeybindProfile *ev)
{
    MapClientSession &session(m_session_store.session_from_event(ev));
    Entity *ent = session.m_ent;

    ent->m_player->m_keybinds.setKeybindProfile(ev->profile);
    //qCDebug(logMapEvents) << "Saving currently selected Keybind Profile. Profile name: " << ev->profile;
}



