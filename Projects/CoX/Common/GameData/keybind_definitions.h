#pragma once
#include <vector>
#include <QtCore/QString>
#include <QtCore/QDebug>

enum ModKeys
{
    NO_MOD    = 0,
    CTRL_MOD  = 1,
    SHIFT_MOD = 2,
    ALT_MOD   = 3,
};

enum KeyName : int32_t
{
    COH_INPUT_INVALID          = 0,
    COH_INPUT_ESCAPE           = 1,
    COH_INPUT_1                = 2,
    COH_INPUT_2                = 3,
    COH_INPUT_3                = 4,
    COH_INPUT_4                = 5,
    COH_INPUT_5                = 6,
    COH_INPUT_6                = 7,
    COH_INPUT_7                = 8,
    COH_INPUT_8                = 9,
    COH_INPUT_9                = 10,
    COH_INPUT_0                = 11,
    COH_INPUT_MINUS            = 12,
    COH_INPUT_EQUALS           = 13,
    COH_INPUT_BACKSPACE        = 14,
    COH_INPUT_TAB              = 15,
    COH_INPUT_Q                = 16,
    COH_INPUT_W                = 17,
    COH_INPUT_E                = 18,
    COH_INPUT_R                = 19,
    COH_INPUT_T                = 20,
    COH_INPUT_Y                = 21,
    COH_INPUT_U                = 22,
    COH_INPUT_I                = 23,
    COH_INPUT_O                = 24,
    COH_INPUT_P                = 25,
    COH_INPUT_LBRACKET         = 26,
    COH_INPUT_RBRACKET         = 27,
    COH_INPUT_RETURN           = 28,
    COH_INPUT_LCONTROL         = 29,
    COH_INPUT_A                = 30,
    COH_INPUT_S                = 31,
    COH_INPUT_D                = 32,
    COH_INPUT_F                = 33,
    COH_INPUT_G                = 34,
    COH_INPUT_H                = 35,
    COH_INPUT_J                = 36,
    COH_INPUT_K                = 37,
    COH_INPUT_L                = 38,
    COH_INPUT_SEMICOLON        = 39,
    COH_INPUT_APOSTROPHE       = 40,
    COH_INPUT_TILDE            = 41,
    COH_INPUT_LSHIFT           = 42,
    COH_INPUT_BACKSLASH        = 43,
    COH_INPUT_Z                = 44,
    COH_INPUT_X                = 45,
    COH_INPUT_C                = 46,
    COH_INPUT_V                = 47,
    COH_INPUT_B                = 48,
    COH_INPUT_N                = 49,
    COH_INPUT_M                = 50,
    COH_INPUT_COMMA            = 51,
    COH_INPUT_PERIOD           = 52,
    COH_INPUT_SLASH            = 53,
    COH_INPUT_RSHIFT           = 54,
    COH_INPUT_MULTIPLY         = 55,
    COH_INPUT_SPACE            = 57,
    COH_INPUT_F1               = 59,
    COH_INPUT_F2               = 60,
    COH_INPUT_F3               = 61,
    COH_INPUT_F4               = 62,
    COH_INPUT_F5               = 63,
    COH_INPUT_F6               = 64,
    COH_INPUT_F7               = 65,
    COH_INPUT_F8               = 66,
    COH_INPUT_F9               = 67,
    COH_INPUT_F10              = 68,
    COH_INPUT_NUMLOCK          = 69,
    COH_INPUT_SCROLL           = 70,
    COH_INPUT_NUMPAD7          = 71,
    COH_INPUT_NUMPAD8          = 72,
    COH_INPUT_NUMPAD9          = 73,
    COH_INPUT_NUMPAD4          = 75,
    COH_INPUT_NUMPAD5          = 76,
    COH_INPUT_NUMPAD6          = 77,
    COH_INPUT_NUMPAD1          = 79,
    COH_INPUT_NUMPAD2          = 80,
    COH_INPUT_NUMPAD3          = 81,
    COH_INPUT_NUMPAD0          = 82,
    COH_INPUT_F11              = 87,
    COH_INPUT_F12              = 88,
    COH_INPUT_F13              = 100,
    COH_INPUT_F14              = 101,
    COH_INPUT_F15              = 102,
    COH_INPUT_NUMPADEQUALS     = 141,
    COH_INPUT_ABNT_C1          = 115,
    COH_INPUT_ABNT_C2          = 126,
    COH_INPUT_COLON            = 146,
    COH_INPUT_UNDERLINE        = 147,
    COH_INPUT_NEXTTRACK        = 153,
    COH_INPUT_NUMPADENTER      = 156,
    COH_INPUT_RCONTROL         = 157,
    COH_INPUT_MUTE             = 160,
    COH_INPUT_CALCULATOR       = 161,
    COH_INPUT_PLAYPAUSE        = 162,
    COH_INPUT_MEDIASTOP        = 164,
    COH_INPUT_VOLUMEDOWN       = 174,
    COH_INPUT_VOLUMEUP         = 176,
    COH_INPUT_WEBHOME          = 178,
    COH_INPUT_NUMPADCOMMA      = 179,
    COH_INPUT_SYSRQ            = 183,
    COH_INPUT_PAUSE            = 197,
    COH_INPUT_HOME             = 199,
    COH_INPUT_END              = 207,
    COH_INPUT_INSERT           = 210,
    COH_INPUT_DELETE           = 211,
    COH_INPUT_WEBSEARCH        = 229,
    COH_INPUT_WEBFAVORITES     = 230,
    COH_INPUT_WEBREFRESH       = 231,
    COH_INPUT_WEBSTOP          = 232,
    COH_INPUT_WEBFORWARD       = 233,
    COH_INPUT_WEBBACK          = 234,
    COH_INPUT_MYCOMPUTER       = 235,
    COH_INPUT_MAIL             = 236,
    COH_INPUT_MEDIASELECT      = 237,
    COH_INPUT_NUMPADSTAR       = 55,
    COH_INPUT_LALT             = 56,
    COH_INPUT_CAPSLOCK         = 58,
    COH_INPUT_NUMPADMINUS      = 74,
    COH_INPUT_NUMPADPLUS       = 78,
    COH_INPUT_NUMPADPERIOD     = 83,
    COH_INPUT_NUMPADSLASH      = 181,
    COH_INPUT_RALT             = 184,
    COH_INPUT_UPARROW          = 200,
    COH_INPUT_PGUP             = 201,
    COH_INPUT_LEFTARROW        = 203,
    COH_INPUT_RIGHTARROW       = 205,
    COH_INPUT_DOWNARROW        = 208,
    COH_INPUT_PGDN             = 209,
    COH_INPUT_LBUTTON          = 238,
    COH_INPUT_MBUTTON          = 239,
    COH_INPUT_RBUTTON          = 240,
    COH_INPUT_BUTTON4          = 241,
    COH_INPUT_BUTTON5          = 242,
    COH_INPUT_BUTTON6          = 243,
    COH_INPUT_BUTTON7          = 244,
    COH_INPUT_BUTTON8          = 245,
    COH_INPUT_MOUSE_SCROLL     = 246,
    COH_INPUT_JOY1             = 90,
    COH_INPUT_JOY2             = 91,
    COH_INPUT_JOY3             = 92,
    COH_INPUT_JOY4             = 93,
    COH_INPUT_JOY5             = 94,
    COH_INPUT_JOY6             = 95,
    COH_INPUT_JOY7             = 96,
    COH_INPUT_JOY8             = 97,
    COH_INPUT_JOY9             = 98,
    COH_INPUT_JOY10            = 99,
    COH_INPUT_JOY11            = 127,
    COH_INPUT_JOY12            = 128,
    COH_INPUT_JOY13            = 129,
    COH_INPUT_JOY14            = 130,
    COH_INPUT_JOY15            = 131,
    COH_INPUT_JOY16            = 132,
    COH_INPUT_JOY17            = 133,
    COH_INPUT_JOY18            = 134,
    COH_INPUT_JOY19            = 135,
    COH_INPUT_JOY20            = 136,
    COH_INPUT_JOY21            = 137,
    COH_INPUT_JOY22            = 138,
    COH_INPUT_JOY23            = 139,
    COH_INPUT_JOY24            = 140,
    COH_INPUT_JOY25            = 89,
    COH_INPUT_JOYPAD_UP        = 185,
    COH_INPUT_JOYPAD_DOWN      = 186,
    COH_INPUT_JOYPAD_LEFT      = 187,
    COH_INPUT_JOYPAD_RIGHT     = 188,
    COH_INPUT_POV1_UP          = 103,
    COH_INPUT_POV1_DOWN        = 104,
    COH_INPUT_POV1_LEFT        = 105,
    COH_INPUT_POV1_RIGHT       = 113,
    COH_INPUT_POV2_UP          = 114,
    COH_INPUT_POV2_DOWN        = 116,
    COH_INPUT_POV2_LEFT        = 117,
    COH_INPUT_POV2_RIGHT       = 118,
    COH_INPUT_POV3_UP          = 119,
    COH_INPUT_POV3_DOWN        = 122,
    COH_INPUT_POV3_LEFT        = 124,
    COH_INPUT_POV3_RIGHT       = 142,
    COH_INPUT_JOYSTICK1_UP     = 189,
    COH_INPUT_JOYSTICK1_DOWN   = 190,
    COH_INPUT_JOYSTICK1_LEFT   = 191,
    COH_INPUT_JOYSTICK1_RIGHT  = 192,
    COH_INPUT_JOYSTICK2_UP     = 165,
    COH_INPUT_JOYSTICK2_DOWN   = 166,
    COH_INPUT_JOYSTICK2_LEFT   = 167,
    COH_INPUT_JOYSTICK2_RIGHT  = 168,
    COH_INPUT_JOYSTICK3_UP     = 143,
    COH_INPUT_JOYSTICK3_DOWN   = 152,
    COH_INPUT_JOYSTICK3_LEFT   = 154,
    COH_INPUT_JOYSTICK3_RIGHT  = 155,
    COH_INPUT_LAST_NON_GENERIC = 256,
    COH_KEY_GENERIC_CONTROL    = 0x801, // either of control keys
    COH_KEY_GENERIC_SHIFT      = 0x802, // either of shift keys
    COH_KEY_GENERIC_ALT        = 0x803, // either of alt keys
    COH_MOUSE_MOVE             = 0x1000,
};
struct Keybind
{
    KeyName actualKey;
    QString Key;
    QString Command;
};

struct KeyProfiles_Entry
{
    QString DisplayName;
    QString Name;
    std::vector<Keybind> KeybindArr;
};

struct CommandEntry {
    KeyName keyname;
    ModKeys key_mods; // Mod  0, 1 - CONTROL_KEY_PRESSED, 2 - SHIFT PRESSED, 3 - ALT PRESSED
};
struct Command
{
    QString CmdString;
    QString DisplayName;
    CommandEntry bound_to[2];
};
struct CommandCategory_Entry
{
    QString DisplayName;
    std::vector<Command> commands;
};

using Parse_AllKeyProfiles = std::vector<KeyProfiles_Entry>;
using Parse_AllCommandCategories = std::vector<CommandCategory_Entry>;

struct KeybindData {
    KeybindData() : key(), mods(NO_MOD), command() {}
    KeybindData(int k, int m, QString c)
            : key(k), mods(m), command(c) {}
    KeybindData(int k, QString c)
            : key(k), mods(NO_MOD), command(c) {}

    int key;
    int mods;
    QString command;
};

class Keybinds
{
public:
    Keybinds() { binds.resize(256); }

    std::vector<KeybindData> binds = {
        KeybindData(COH_INPUT_APOSTROPHE,"quickchat"),              // (        Qt::Key_ParenLeft
        KeybindData(COH_INPUT_MINUS,"prev_tray"),                   // -        Qt::Key_Minus
        KeybindData(COH_INPUT_MINUS,ALT_MOD,"prev_tray_alt"),       // ALT+-    Qt::AltModifier + Qt::Key_Minus
        KeybindData(COH_INPUT_SLASH,"show chat$$slashchat"),        // /        Qt::Key_Slash
        KeybindData(COH_INPUT_0,"powexec_slot 10"),                 // 0    Qt::Key_0
        KeybindData(COH_INPUT_1,"powexec_slot 1"),                  // 1    Qt::Key_1
        KeybindData(COH_INPUT_2,"powexec_slot 2"),                  // 2    Qt::Key_2
        KeybindData(COH_INPUT_3,"powexec_slot 3"),                  // 3    Qt::Key_3
        KeybindData(COH_INPUT_4,"powexec_slot 4"),                  // 4    Qt::Key_4
        KeybindData(COH_INPUT_5,"powexec_slot 5"),                  // 5    Qt::Key_5
        KeybindData(COH_INPUT_6,"powexec_slot 6"),                  // 6    Qt::Key_6
        KeybindData(COH_INPUT_7,"powexec_slot 7"),                  // 7    Qt::Key_7
        KeybindData(COH_INPUT_8,"powexec_slot 8"),                  // 8    Qt::Key_8
        KeybindData(COH_INPUT_9,"powexec_slot 9"),                  // 9    Qt::Key_9
        //KeybindData(COH_INPUT_0,SHIFT_MOD,"unassigned"),          // SHIFT+0   Qt::ShiftModifier + Qt::Key_0
        KeybindData(COH_INPUT_1,SHIFT_MOD,"team_select 1"),         // SHIFT+1   Qt::ShiftModifier + Qt::Key_1
        KeybindData(COH_INPUT_2,SHIFT_MOD,"team_select 2"),         // SHIFT+2   Qt::ShiftModifier + Qt::Key_2
        KeybindData(COH_INPUT_3,SHIFT_MOD,"team_select 3"),         // SHIFT+3   Qt::ShiftModifier + Qt::Key_3
        KeybindData(COH_INPUT_4,SHIFT_MOD,"team_select 4"),         // SHIFT+4   Qt::ShiftModifier + Qt::Key_4
        KeybindData(COH_INPUT_5,SHIFT_MOD,"team_select 5"),         // SHIFT+5   Qt::ShiftModifier + Qt::Key_5
        KeybindData(COH_INPUT_6,SHIFT_MOD,"team_select 6"),         // SHIFT+6   Qt::ShiftModifier + Qt::Key_6
        KeybindData(COH_INPUT_7,SHIFT_MOD,"team_select 7"),         // SHIFT+7   Qt::ShiftModifier + Qt::Key_7
        KeybindData(COH_INPUT_8,SHIFT_MOD,"team_select 8"),         // SHIFT+8   Qt::ShiftModifier + Qt::Key_8
        //KeybindData(COH_INPUT_9,SHIFT_MOD,"unassigned"),          // SHIFT+9   Qt::ShiftModifier + Qt::Key_9
        KeybindData(COH_INPUT_0,CTRL_MOD,"powexec_alt2slot 10"),    // CTRL+0   Qt::ControlModifier + Qt::Key_0
        KeybindData(COH_INPUT_1,CTRL_MOD,"powexec_alt2slot 1"),     // CTRL+1   Qt::ControlModifier + Qt::Key_1
        KeybindData(COH_INPUT_2,CTRL_MOD,"powexec_alt2slot 2"),     // CTRL+2   Qt::ControlModifier + Qt::Key_2
        KeybindData(COH_INPUT_3,CTRL_MOD,"powexec_alt2slot 3"),     // CTRL+3   Qt::ControlModifier + Qt::Key_3
        KeybindData(COH_INPUT_4,CTRL_MOD,"powexec_alt2slot 4"),     // CTRL+4   Qt::ControlModifier + Qt::Key_4
        KeybindData(COH_INPUT_5,CTRL_MOD,"powexec_alt2slot 5"),     // CTRL+5   Qt::ControlModifier + Qt::Key_5
        KeybindData(COH_INPUT_6,CTRL_MOD,"powexec_alt2slot 6"),     // CTRL+6   Qt::ControlModifier + Qt::Key_6
        KeybindData(COH_INPUT_7,CTRL_MOD,"powexec_alt2slot 7"),     // CTRL+7   Qt::ControlModifier + Qt::Key_7
        KeybindData(COH_INPUT_8,CTRL_MOD,"powexec_alt2slot 8"),     // CTRL+8   Qt::ControlModifier + Qt::Key_8
        KeybindData(COH_INPUT_9,CTRL_MOD,"powexec_alt2slot 9"),     // CTRL+9   Qt::ControlModifier + Qt::Key_9
        KeybindData(COH_INPUT_0,ALT_MOD,"powexec_altslot 10"),      // ALT+0    Qt::Key_ParenRight
        KeybindData(COH_INPUT_1,ALT_MOD,"powexec_altslot 1"),       // ALT+1    Qt::Key_Exclam
        KeybindData(COH_INPUT_2,ALT_MOD,"powexec_altslot 2"),       // ALT+2    Qt::Key_At
        KeybindData(COH_INPUT_3,ALT_MOD,"powexec_altslot 3"),       // ALT+3    Qt::Key_NumberSign
        KeybindData(COH_INPUT_4,ALT_MOD,"powexec_altslot 4"),       // ALT+4    Qt::Key_Dollar
        KeybindData(COH_INPUT_5,ALT_MOD,"powexec_altslot 5"),       // ALT+5    Qt::Key_Percent
        KeybindData(COH_INPUT_6,ALT_MOD,"powexec_altslot 6"),       // ALT+6    Qt::Key_AsciiCircum
        KeybindData(COH_INPUT_7,ALT_MOD,"powexec_altslot 7"),       // ALT+7    Qt::Key_Ampersand
        KeybindData(COH_INPUT_8,ALT_MOD,"powexec_altslot 8"),       // ALT+8    Qt::Key_Asterisk
        KeybindData(COH_INPUT_9,ALT_MOD,"powexec_altslot 9"),       // ALT+9    Qt::Key_ParenLeft
        KeybindData(COH_INPUT_SEMICOLON,"show chat$$beginchat ;"),  // ;    Qt::Key_Semicolon
        KeybindData(COH_INPUT_BACKSLASH,"menu"),                    // \    Qt::Key_Backslash
        KeybindData(COH_INPUT_A,"+left"),                           // A    Qt::Key_A
        KeybindData(COH_INPUT_B,"++first"),                         // B    Qt::Key_B
        KeybindData(COH_INPUT_C,"chat"),                            // C    Qt::Key_C
        KeybindData(COH_INPUT_D,"+right"),                          // D    Qt::Key_D
        KeybindData(COH_INPUT_E,"+turnright"),                      // E    Qt::Key_E
        KeybindData(COH_INPUT_F,"follow"),                          // F    Qt::Key_F
        //KeybindData(COH_INPUT_G,"unassigned"),                    // G    Qt::Key_G
        KeybindData(COH_INPUT_H,"helpwindow"),                      // H    Qt::Key_H
        //KeybindData(COH_INPUT_I,"unassigned"),                    // I    Qt::Key_I
        //KeybindData(COH_INPUT_J,"unassigned"),                    // J    Qt::Key_J
        //KeybindData(COH_INPUT_K,"unassigned"),                    // K    Qt::Key_K
        //KeybindData(COH_INPUT_L,"unassigned"),                    // L    Qt::Key_L
        KeybindData(COH_INPUT_M,"map"),                             // M    Qt::Key_M
        KeybindData(COH_INPUT_N,"nav"),                             // N    Qt::Key_N
        //KeybindData(COH_INPUT_O,"unassigned"),                    // O    Qt::Key_O
        KeybindData(COH_INPUT_P,"powers"),                          // P    Qt::Key_P
        KeybindData(COH_INPUT_Q,"+turnleft"),                       // Q    Qt::Key_Q
        KeybindData(COH_INPUT_R,"++autorun"),                       // R    Qt::Key_R
        KeybindData(COH_INPUT_S,"+backward"),                       // S    Qt::Key_S
        KeybindData(COH_INPUT_T,"target"),                          // T    Qt::Key_T
        //KeybindData(COH_INPUT_U,"unassigned"),                    // U    Qt::Key_U
        KeybindData(COH_INPUT_V,"+ctm_invert"),                     // V    Qt::Key_V
        KeybindData(COH_INPUT_W,"+forward"),                        // W    Qt::Key_W
        KeybindData(COH_INPUT_X,"+down"),                           // X    Qt::Key_X
        //KeybindData(COH_INPUT_Y,"unassigned"),                    // Y    Qt::Key_Y
        KeybindData(COH_INPUT_Z,"powerexec_abort"),                 // Z    Qt::Key_Z
        KeybindData(COH_INPUT_ESCAPE,"unselect"),                   // escape       Qt::Key_Escape
        KeybindData(COH_INPUT_TAB,"toggle_enemy"),                  // tab          Qt::Key_Tab
        KeybindData(COH_INPUT_TAB,SHIFT_MOD,"target_enemy_prev"),   // SHIFT+TAB      Qt::Key_Backtab
        KeybindData(COH_INPUT_TAB,CTRL_MOD,"target_enemy_near"),    // CTRL+TAB    Qt::ControlModifier + Qt::Key_Tab
        KeybindData(COH_INPUT_BACKSPACE,"autoreply"),               // backspace    Qt::Key_Backspace
        KeybindData(COH_INPUT_RETURN,"show chat$$startchat"),       // return       Qt::Key_Return
        //KeybindData(COH_INPUT_RETURN,"unassigned"),               // enter        Qt::Key_Enter
        KeybindData(COH_INPUT_INSERT,"+lookup"),                    // insert       Qt::Key_Insert
        KeybindData(COH_INPUT_DELETE,"+lookdown"),                  // delete       Qt::Key_Delete
        KeybindData(COH_INPUT_PAUSE,"++screenshotui"),              // pause        Qt::Key_Pause
        //KeybindData(COH_INPUT_PRINT,"unassigned"),                // print        Qt::Key_Print
        KeybindData(COH_INPUT_SYSRQ,"screenshot"),                  // sysreq       Qt::Key_SysReq
        //KeybindData(COH_INPUT_CLEAR,"unassigned"),                // clear        Qt::Key_Clear
        KeybindData(COH_INPUT_HOME,"+zoomin"),                      // home         Qt::Key_Home
        KeybindData(COH_INPUT_END,"+zoomout"),                      // end          Qt::Key_End
        KeybindData(COH_INPUT_LEFTARROW,"+turnleft"),               // left         Qt::Key_Left
        KeybindData(COH_INPUT_UPARROW,"+forward"),                  // up           Qt::Key_Up
        KeybindData(COH_INPUT_RIGHTARROW,"+turnright"),             // right        Qt::Key_Right
        KeybindData(COH_INPUT_DOWNARROW,"+backward"),               // down         Qt::Key_Down
        KeybindData(COH_INPUT_PGUP,"+camrotate"),                   // pageup       Qt::Key_PageUp
        KeybindData(COH_INPUT_PGDN,"camreset"),                     // pagedown     Qt::Key_PageDown
        //KeybindData(COH_INPUT_LSHIFT,"unassigned"),               // shift        Qt::Key_Shift
        KeybindData(COH_INPUT_LCONTROL,"+alt2tray"),                // leftcontrol  Qt::Key_Control
        //KeybindData(0x01000022,"unassigned"),                     // windowskey   Qt::Key_Meta
        KeybindData(COH_INPUT_F1,"inspexec_slot 1"),                // F1   Qt::Key_F1
        KeybindData(COH_INPUT_F2,"inspexec_slot 2"),                // F2   Qt::Key_F2
        KeybindData(COH_INPUT_F3,"inspexec_slot 3"),                // F3   Qt::Key_F3
        KeybindData(COH_INPUT_F4,"inspexec_slot 4"),                // F4   Qt::Key_F4
        KeybindData(COH_INPUT_F5,"inspexec_slot 5"),                // F5   Qt::Key_F5
        KeybindData(COH_INPUT_F6,"local <color white><bgcolor red>RUN!"),                           // F6   Qt::Key_F6
        KeybindData(COH_INPUT_F7,"say <color black><bgcolor #22aa22>Ready! $$ emote thumbsup"),     // F7   Qt::Key_F7
        KeybindData(COH_INPUT_F8,"local <color black><bgcolor #aaaa22>HELP! $$ emote whistle"),     // F8   Qt::Key_F8
        KeybindData(COH_INPUT_F9,"local <color white><bgcolor #2222aa><scale .75>level $level $archetype$$local <color white><bgcolor #2222aa>Looking for team"),   // F9   Qt::Key_F9
        KeybindData(COH_INPUT_F10,"say $battlecry $$ emote attack"),            // F10  Qt::Key_F10
        //KeybindData(COH_INPUT_F11,"unassigned"),                              // F11   Qt::Key_F11
        //KeybindData(COH_INPUT_F12,"unassigned"),                              // F12   Qt::Key_F12
        KeybindData(COH_INPUT_LEFTARROW,"+turnleft"),                           // directionleft    Qt::Key_Direction_L
        KeybindData(COH_INPUT_RIGHTARROW,"+turnright"),                         // directionright   Qt::Key_Direction_R
        KeybindData(COH_INPUT_SPACE,"+up"),                                     // space            Qt::Key_Space
        KeybindData(COH_INPUT_COMMA,"show chat$$beginchat /tell $target, "),    // ,        Qt::Key_Comma
        KeybindData(COH_INPUT_EQUALS,"next_tray"),                              // =        Qt::Key_Equal
        KeybindData(COH_INPUT_EQUALS,ALT_MOD,"next_tray_alt"),                  // ALT+=    Qt::Key_Equal
        KeybindData(COH_INPUT_MBUTTON,"+camrotate"),                            // MBUTTON
        KeybindData(COH_INPUT_LBUTTON,COH_INPUT_RBUTTON,"+forward_mouse"),      // MouseChord "left and right"
        KeybindData(COH_INPUT_MOUSE_SCROLL,"+camdistadjust"),                   // MOUSEWHEEL
        KeybindData(COH_INPUT_RALT,"+alttreysticky"),                           // Right ALT
        KeybindData(COH_INPUT_RBUTTON,"+mouse_look"),                           // right button
        KeybindData(COH_INPUT_SPACE,"+up"),                                     // space    Qt::Key_Space
    };

    // Keybind Members
    QString m_cur_keybind_profile = "Default";

    void keybindsDump()
    {
        qDebug().noquote() << "Debugging Keybinds:"
                           << "\n\t" << "Profile Name:" << m_cur_keybind_profile;

        for(auto k : binds)
            qDebug() << k.key << k.mods << k.command;
    }
};
