/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "clientoptions_definitions.h"
#include "gui_definitions.h"
#include "keybind_definitions.h"

#include <stdint.h>

// this part of the Entity is for Players only info
struct PlayerData
{
    static const constexpr  uint32_t    class_version   = 2;
    GUISettings         m_gui;
    KeybindSettings     m_keybinds;
    ClientOptions       m_options;
    uint8_t             m_auth_data[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}; // preorder rewards and things
    void reset()
    {
        m_options.m_first_person_view=false;
    }
    void dump() const
    {
        m_options.clientOptionsDump();
    }
};
