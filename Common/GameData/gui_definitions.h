/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <stdint.h>
#include <vector>
#include <array>
#include <cassert>

#include <QDebug>
#include <cereal/cereal.hpp>

enum WindowIDX : uint32_t {
    wdw_DockDraw        = 0,
    wdw_Status          = 1,
    wdw_Target          = 2,
    wdw_PowersTray      = 3,
    wdw_Chat            = 4,
    wdw_PowerList       = 5,
    wdw_Team            = 6,
    wdw_NavCompass      = 7,
    wdw_Map             = 8,
    wdw_ChatOptions     = 9,
    wdw_Friends         = 10,
    wdw_ContactDiag     = 11,
    wdw_Inspirations    = 12,
    wdw_SuperGroup      = 13,
    wdw_Emails          = 14,
    wdw_ComposeEmail    = 15,
    wdw_Contacts        = 16,
    wdw_Missions        = 17,
    wdw_Clues           = 18,
    wdw_Trade           = 19,
    wdw_Quit            = 20,
    wdw_Info            = 21,
    wdw_Help            = 22,
    wdw_MissionSummary  = 23,
    wdw_Actions         = 24,
    wdw_Browser         = 25,
    wdw_Search          = 26,
    wdw_Shop            = 27,
    wdw_GenericDlg      = 28,
    wdw_MissionFeedback = 29,
    wdw_Support         = 30,
    wdw_TitleSelect     = 31,
    wdw_Defeated        = 32,
    wdw_MapXferList     = 33,
    wdw_CostumeSelect   = 34,
};

enum WindowVisibility : uint32_t {
    wv_Uninitialized    = 0,
    wv_Growing          = 1,
    wv_Visible          = 2,
    wv_Shrinking        = 3,
    wv_DockedOrHidden   = 4,
};

enum ChatWindowMasks : uint32_t {   // top      bottom  bottom              top
    ch_TopMask          = 0x4001,   // 16385    0
    ch_Damage           = 0x4,      // 16389    4       0000000000000000100 0000100000000000101
    ch_Combat           = 0x2,      // 16387    2       0000000000000000010 0000100000000000011
    ch_System           = 0x18048,  // 114761   98376   0011000000001001000 0011100000001001001
    ch_NPCDialog        = 0x40030,  // 278577   262192  1000000000000110000 1000100000000110001
    ch_PrivateMsg       = 0x80,     // 16513    128     0000000000010000000 0000100000010000001
    ch_Team             = 0x100,    // 16641    256     0000000000100000000 0000100000100000001
    ch_SuperGroup       = 0x200,    // 16897    512     0000000001000000000 0000100001000000001
    ch_Local            = 0x400,    // 17409    1024    0000000010000000000 0000100010000000001
    ch_Broadcast        = 0x800,    // 18433    2048    0000000100000000000 0000100100000000001
    ch_Request          = 0x1000,   // 20481    4096    0000001000000000000 0000101000000000001
    ch_Friends          = 0x2000,   // 24577    8192    0000010000000000000 0000110000000000001
    ch_Emotes           = 0x20000,  // 147457   131072  0100000000000000000 0100100000000000001
};

class GUIWindow
{
public:
        // GUIWindow serialization class version
        enum : uint32_t {class_version = 1};

        // GUI Window Params
        WindowIDX           m_idx               = wdw_DockDraw;
        WindowVisibility    m_mode              = wv_Uninitialized;
        int32_t             m_posx              = 0;
        int32_t             m_posy              = 0;
        uint32_t            m_width             = 0;
        uint32_t            m_height            = 0;
        uint32_t            m_locked            = 0;
        uint32_t            m_color             = 0x3399FF99;   // 0x3399FF99 (light blue with 60% transparency)
        uint32_t            m_alpha             = 0x88;         // default 136 (0x88)
        bool                m_draggable_frame   = false;

        void                guiWindowDump() const
                            {
                                qDebug().noquote() << "GUIWindow:" << m_idx
                                         << "\n\t" << "posx:" << m_posx
                                         << "\n\t" << "posy:" << m_posy
                                         << "\n\t" << "width:" << m_width
                                         << "\n\t" << "height:" << m_height
                                         << "\n\t" << "draggable_frame:" << m_draggable_frame
                                         << "\n\t" << "locked:" << m_locked
                                         << "\n\t" << "mode:" << m_mode
                                         << "\n\t" << "color:" << m_color
                                         << "\n\t" << "alpha:" << m_alpha;
                            }

        void                setWindowVisibility(WindowVisibility val) { m_mode = val; }
        template<class Archive>
        void                serialize(Archive &archive)
                            {
                                archive(cereal::make_nvp("IDX",m_idx));
                                archive(cereal::make_nvp("Mode",m_mode));
                                archive(cereal::make_nvp("DraggableFrame",m_draggable_frame));
                                archive(cereal::make_nvp("PosX",m_posx));
                                archive(cereal::make_nvp("PosY",m_posy));
                                archive(cereal::make_nvp("Width",m_width));
                                archive(cereal::make_nvp("Height",m_height));
                                archive(cereal::make_nvp("Locked",m_locked));
                                archive(cereal::make_nvp("Color",m_color));
                                archive(cereal::make_nvp("Alpha",m_alpha));
                            }
};

class GUISettings
{
public:
    GUISettings() = default;

        // GUISettings serialization class version
        enum : uint32_t { class_version = 1 };

        // List of Windows
        std::array<GUIWindow, 35> m_wnds;

        // Other GUI flags
        bool        m_team_buffs        = false;
        uint8_t     m_cur_chat_channel  = 10;       // Default is local (10)
        bool        m_powers_tray_mode  = false;
        bool        m_insps_tray_mode   = false;
        int         m_tray1_number      = 1;
        int         m_tray2_number      = 2;
        int         m_tray3_number      = 3;
        int         m_chat_top_flags    = 0x5D87F; // orig: (1<<19)-1
        int         m_chat_bottom_flags = 0x22780;
        float       m_chat_divider_pos  = 0.5f;

        void guiDump() const
        {
            qDebug().noquote() << "Debugging GUISettings:"
                               << "\n\t" << "TeamBuffs:" << m_team_buffs
                               << "\n\t" << "ChatChannel:" << m_cur_chat_channel
                               << "\n\t" << "PowersTray Mode:" << m_powers_tray_mode
                               << "\n\t" << "InspTray Mode:" << m_insps_tray_mode
                               << "\n\t" << "Tray1 Page:" << m_tray1_number
                               << "\n\t" << "Tray2 Page:" << m_tray2_number
                               << "\n\t" << "Tray3 Page:" << m_tray3_number
                               << "\n\t" << "Chat Window Masks Top:" << m_chat_top_flags
                               << "\n\t" << "Chat Window Masks Bottom:" << m_chat_bottom_flags
                               << "\n\t" << "Chat Divider Position:" << m_chat_divider_pos;

            for(const auto &wnd : m_wnds)
                wnd.guiWindowDump();
        }
};
