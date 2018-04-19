#pragma once

#include "clientoptions_definitions.h"
#include "gui_definitions.h"
#include "keybind_definitions.h"

#include <stdint.h>

// this part of the Entity is for Players only info
struct PlayerData
{
    static const constexpr  uint32_t    class_version       = 1;
    GUISettings         m_gui;
    KeybindSettings     m_keybinds;
    ClientOptions       m_options;
    void reset()
    {
        m_options.m_first_person_view=false;
    }
    void dump() const
    {
        m_options.clientOptionsDump();
    }
};
