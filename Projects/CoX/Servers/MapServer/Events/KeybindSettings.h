/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "Common/GameData/keybind_definitions.h"

class KeybindSettings
{
public:
    KeybindSettings();

// Keybind serialization class version
static const constexpr  uint32_t class_version = 1;

        // Keybind Members
        Parse_AllKeyProfiles m_keybind_profiles;
        QString m_cur_keybind_profile = "Default";

        // Keybind Methods
        void setKeybindProfile(QString &profile);
const   CurrentKeybinds &getCurrentKeybinds() const;
        void resetKeybinds();
        void setKeybind(QString &profile, KeyName &key, ModKeys &mods, QString &command, bool &is_secondary);
        void removeKeybind(QString &profile, KeyName &key, ModKeys &mods);
        void keybindsDump();
};
