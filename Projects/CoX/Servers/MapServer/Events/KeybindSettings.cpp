/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "KeybindSettings.h"
#include "MapServer.h"
#include "MapServerData.h"

QString makeKeyString(const KeyName &key, const KeyName &mods)
{
    QString k = keyNameToEnum.key(key);
    QString m = keyNameToEnum.key(mods);
    QString keystring;

    if(m != "\u0000")
        keystring = QString("%1+%2").arg(m,k);
    else
        keystring = k;

    qDebug() << keystring << k << m;

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
    for(auto &p : m_keybind_profiles)
    {
        if(p.Name == m_cur_keybind_profile)
            return p.KeybindArr;
    }

    assert(false);
    return m_keybind_profiles.at(0).KeybindArr;
}

void KeybindSettings::resetKeybinds()
{
    const MapServerData &data(g_GlobalMapServer->runtimeData());
    const Parse_AllKeyProfiles &default_profiles(data.m_keybind_profiles);

    m_keybind_profiles = default_profiles;
}

void KeybindSettings::setKeybind(QString &profile, KeyName &key, KeyName &mods, QString &command, bool &is_secondary)
{
    QString keystring = makeKeyString(key,mods); // TODO: correct keystring
    qDebug() << profile << key << mods << keystring << command << is_secondary;

    for(auto &p : m_keybind_profiles)
    {
        if(p.Name == profile)
            p.KeybindArr.push_back({key,mods,keystring,command,is_secondary});
    }
}

void KeybindSettings::removeKeybind(QString &profile, KeyName &key, KeyName &mods)
{
    // TODO: find and remove keybind here
}

void KeybindSettings::keybindsDump()
{
    qDebug().noquote() << "Debugging Keybinds:"
                       << "\n\t" << "Current Profile Name:" << m_cur_keybind_profile;

    for(auto &profile : m_keybind_profiles)
    {
        qDebug() << profile.DisplayName << profile.Name;

        for(auto &k : profile.KeybindArr)
            qDebug() << k.KeyString << k.Key << k.Mods << k.Command << k.IsSecondary;
    }
}
