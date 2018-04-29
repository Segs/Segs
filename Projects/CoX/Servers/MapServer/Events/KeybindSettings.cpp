/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include <algorithm>

#include "KeybindSettings.h"
#include "MapServer.h"
#include "MapServerData.h"
#include "Logging.h"

QString makeKeyString(const KeyName &key, const ModKeys &mods)
{
    QString k = keyNameToEnum.key(key,0);
    QString m = modNameToEnum.key(mods,0);
    QString keystring;

    if(m != 0)
        keystring = QString("%1+%2").arg(m,k);
    else
        keystring = k;

    //qCDebug(logKeybinds) << keystring << k << m << key << mods;

    return keystring.toLower();
}

KeybindSettings::KeybindSettings()
{
    resetKeybinds();
}

void KeybindSettings::setKeybindProfile(QString &profile)
{
    m_cur_keybind_profile = profile;
}

const CurrentKeybinds &KeybindSettings::getCurrentKeybinds() const
{
    for(const auto &p : m_keybind_profiles)
    {
        if(p.Name == m_cur_keybind_profile)
            return p.KeybindArr;
    }

    qCDebug(logKeybinds) << "Could not get Current Keybinds. Returning first keybind profile.";
    return m_keybind_profiles.at(0).KeybindArr;
}

void KeybindSettings::resetKeybinds()
{
    const MapServerData &data(g_GlobalMapServer->runtimeData());
    const Parse_AllKeyProfiles &default_profiles(data.m_keybind_profiles);

    m_keybind_profiles = default_profiles;
}

void KeybindSettings::setKeybind(QString &profile, KeyName &key, ModKeys &mods, QString &command, bool &is_secondary)
{
    removeKeybind(profile,key,mods); // remove previous keybinds

    QString keystring = makeKeyString(key,mods); // Construct keystring from mods+key

    for(auto &p : m_keybind_profiles)
    {
        if(p.Name == profile)
            p.KeybindArr.push_back({key,mods,keystring,command,is_secondary});
    }

    qCDebug(logKeybinds) << "Setting keybind: " << profile << key << mods << keystring << command << is_secondary;

}

void KeybindSettings::removeKeybind(QString &profile, KeyName &key, ModKeys &mods)
{
    QString keystring = makeKeyString(key,mods); // Construct keystring from mods+key

    for(auto &p : m_keybind_profiles)
    {
        if(p.Name != profile)
            continue;

        for(auto iter=p.KeybindArr.begin(); iter!=p.KeybindArr.end(); /*incremented inside loop*/)
        {
          if(iter->Key==key && iter->Mods==mods)
              iter=p.KeybindArr.erase(iter); // this is flagged by Coverity, but is fine since we perform an end check in the for loop.
          else
              ++iter;
        }
    }
    qCDebug(logKeybinds) << "Clearing keybind: " << profile << key << mods << keystring;
}

void KeybindSettings::keybindsDump()
{
    qDebug().noquote() << "Debugging Keybinds:"
                       << "\n\t" << "Current Profile Name:" << m_cur_keybind_profile;

    for(const auto &profile : m_keybind_profiles)
    {
        qDebug() << profile.DisplayName << profile.Name;

        for(const auto &k : profile.KeybindArr)
            qDebug() << k.KeyString << k.Key << k.Mods << k.Command << k.IsSecondary;
    }
}

//! @}
