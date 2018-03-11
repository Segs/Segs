#pragma once

#include <QtCore/QString>
#include <stdint.h>
#include <vector>
#include <cassert>

#include <QDebug>

enum WindowIDX : uint32_t {
    wdw_Unknown0        = 0,
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
    wdw_Unknown11       = 11,
    wdw_Inspirations    = 12,
    wdw_SuperGroup      = 13,
    wdw_Emails          = 14,
    wdw_ComposeEmail    = 15,
    wdw_Contacts        = 16,
    wdw_Missions        = 17,
    wdw_Clues           = 18,
    wdw_Unknown19       = 19,
    wdw_Quit            = 20,
    wdw_Info            = 21,
    wdw_Help            = 22,
    wdw_Unknown23       = 23,
    wdw_Actions         = 24,
    wdw_Unknown25       = 25,
    wdw_Unknown26       = 26,
    wdw_Unknown27       = 27,
    wdw_GenericDlg      = 28,
    wdw_Unknown29       = 29,
    wdw_Support         = 30,
    wdw_Unknown31       = 31,
    wdw_Defeated        = 32,
    wdw_Unknown33       = 33,
    wdw_CostumeSelect   = 34,
};

enum WindowVisibility : uint32_t {
    wv_Uninitialized    = 0,
    wv_Growing          = 1,
    wv_Visible          = 2,
    wv_Shrinking        = 3,
    wv_DockedOrHidden   = 4,
};

class GUIWindow
{
public:
    GUIWindow() { }

        // GUI Window Params
        WindowIDX           m_idx;
        WindowVisibility    m_mode;
        bool                m_draggable_frame;
        uint32_t            m_posx;
        uint32_t            m_posy;
        uint32_t            m_width            = 0;
        uint32_t            m_height           = 0;
        uint32_t            m_locked;
        uint32_t            m_color            = 0x3399FF99;   // 0x3399FF99 (light blue with 90% transparency)
        uint32_t            m_alpha            = 0x88;         // default 136 (0x88)

        void guiWindowDump()
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

};

class GUISettings : private GUIWindow
{
public:
    GUISettings() { }

        // List of Windows
        std::vector<GUIWindow> m_wnds;

        // Other GUI flags
        bool        m_team_buffs        = false;
        uint8_t     m_cur_chat_channel  = 10;       // Default is local (10)
        bool        m_powers_tray_mode  = false;
        bool        m_insps_tray_mode   = false;
        int         m_tray1_number      = 1;
        int         m_tray2_number      = 2;
        int         m_tray3_number      = 3;

        void guiDump()
        {
            qDebug().noquote() << "Debugging GUISettings:"
                     << "\n\t" << "TeamBuffs:" << m_team_buffs;

            for(auto wnd : m_wnds)
                wnd.guiWindowDump();
        }

};
