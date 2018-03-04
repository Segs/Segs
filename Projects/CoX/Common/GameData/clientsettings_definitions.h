#pragma once

struct ClientSettingsData
{
    enum Show : uint8_t {
      HideAlways = 0x0,
      Always = 0x1,
      OnMouseOver = 0x2,
      Selected = 0x4,
    };
    float mouse_speed;
    float turn_speed;
    bool invert_mouse;
    bool win_4_autofade; //TODO: better names for those
    bool win_7_autofade;
    bool show_tooltips;
    bool disable_profanity_filter;
    bool use_chat_baloons;
    Show archetype_visibility;
    Show supergroup_visibility;
    Show player_name;
    Show player_bars;
    Show enemy_name;
    Show enemy_bars;
    Show player_reticles;
    Show enemy_reticles;
    Show assist_reticles;
    int font_size;
};
