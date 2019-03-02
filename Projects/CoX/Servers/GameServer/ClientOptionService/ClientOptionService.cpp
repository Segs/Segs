/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "ClientOptionService.h"
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


void ClientOptionService::on_set_keybind(Entity* ent, Event *ev)
{
    SetKeybind* data = static_cast<SetKeybind *>(ev);
    KeyName key = static_cast<KeyName>(data->key);
    ModKeys mod = static_cast<ModKeys>(data->mods);

    ent->m_player->m_keybinds.setKeybind(data->profile, key, mod, data->command, data->is_secondary);
    //qCDebug(logMapEvents) << "Setting keybind: " << ev->profile << QString::number(ev->key) << QString::number(ev->mods) << ev->command << ev->is_secondary;
}


void ClientOptionService::on_remove_keybind(Entity* ent, Event *ev)
{
    RemoveKeybind* data = static_cast<RemoveKeybind *>(ev);
    ent->m_player->m_keybinds.removeKeybind(data->profile,(KeyName &)data->key,(ModKeys &)data->mods);
    //qCDebug(logMapEvents) << "Clearing Keybind: " << ev->profile << QString::number(ev->key) << QString::number(ev->mods);
}

void ClientOptionService::on_reset_keybinds(Entity* ent, Event *ev)
{
    // the event is used to merely get the session and then the kbSettings, which we already found in MapInstance
    const GameDataStore &data(getGameData());
    const Parse_AllKeyProfiles &default_profiles(data.m_keybind_profiles);

    ent->m_player->m_keybinds.resetKeybinds(default_profiles);
    //qCDebug(logMapEvents) << "Resetting Keybinds to defaults.";
}

void ClientOptionService::on_select_keybind_profile(Entity* ent, Event *ev)
{
    SelectKeybindProfile* data = static_cast<SelectKeybindProfile *>(ev);
    ent->m_player->m_keybinds.setKeybindProfile(data->profile);
    //qCDebug(logMapEvents) << "Saving currently selected Keybind Profile. Profile name: " << ev->profile;
}



