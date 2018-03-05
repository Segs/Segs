/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "../MapEvents.h"

enum WindowVisibility : uint32_t {
  wv_HideAlways     = 0,
  wv_Always         = 1,
  wv_OnMouseOver    = 2,
  wv_Selected       = 4,
};

class SaveClientOptions final : public MapLinkEvent
{
public:
    ClientOptions data;
    SaveClientOptions() : MapLinkEvent(MapEventTypes::evSaveClientOptions) {}
    void serializeto(BitStream &bs) const override;
    void serializefrom(BitStream &bs) override;
};

struct ClientOption
{
    enum eType
    {
        t_int           = 1,
        t_string        = 2,
        t_float         = 3,
        t_sentence      = 4,
        t_quant_angle   = 5,
        t_mat4          = 6,
        t_vec3          = 7,
        t_date          = 9,
        t_unknown
    };
    struct Arg
    {
        Arg(int t,void *v) : type(eType(t)),tgt(v){}
        eType type;
        void *tgt;
    };
    std::string name;
    std::vector<Arg> m_args;
    //ClientOption(const char *v) : name(v) {}
};

class ClientOptions
{
    std::vector<ClientOption> m_opts;
    void init();
public:
    ClientOptions()
    {
        init();
        m_mouse_speed = 0.6f;
    }
    int32_t control_debug       = 0;
    int32_t no_strafe           = 0;
    int32_t alwaysmobile        = 0; // 1- player is always mobile (can't be immobilized by powers)
    int32_t repredict           = 0; // 1 - client is out of sync with server, asking for new physics state info.
    int32_t neterrorcorrection  = 0;
    float   speed_scale         = 0;

    int32_t svr_lag,svr_lag_vary,svr_pl,svr_oo_packets,client_pos_id;
    int32_t atest0,atest1,atest2,atest3,atest4,atest5,atest6,atest7,atest8,atest9;
    int32_t predict,notimeout,selected_ent_server_index;

    bool             m_mouse_invert         = 0;
    float            m_mouse_speed          = 0;
    float            m_turn_speed           = 3;
    bool             m_fade_chat_wnd        = 0;
    bool             m_fade_nav_wnd         = 0;
    bool             m_show_tooltips        = true;
    bool             m_allow_profanity      = true;
    bool             m_chat_balloons        = true;
    WindowVisibility m_show_archetype       = wv_OnMouseOver;
    WindowVisibility m_show_supergroup      = wv_OnMouseOver;
    WindowVisibility m_show_player_name     = wv_OnMouseOver;
    WindowVisibility m_show_player_bars     = wv_OnMouseOver;
    WindowVisibility m_show_enemy_name      = wv_OnMouseOver;
    WindowVisibility m_show_enemy_bars      = wv_OnMouseOver;
    WindowVisibility m_show_player_reticles = wv_OnMouseOver;
    WindowVisibility m_show_enemy_reticles  = wv_OnMouseOver;
    WindowVisibility m_show_assist_reticles = wv_OnMouseOver;
    uint8_t          m_chat_font_size       = 0;

    ClientOption *get(int idx)
    {
        if(idx<0)
            return nullptr;
        assert((size_t(idx)<m_opts.size()) && "Unknown option requested!!");
        return &m_opts[idx];
    }

};
