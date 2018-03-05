/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "Servers/MapServer/Events/ClientOptions.h"

//#include "BitStream.h"

void SaveClientOptions::serializefrom(BitStream & bs)
{
    data.m_mouse_speed          = bs.GetFloat();
    data.m_turn_speed           = bs.GetFloat();
    data.m_mouse_invert         = bs.GetBits(1);
    data.m_fade_chat_wnd        = bs.GetBits(1);
    data.m_fade_nav_wnd         = bs.GetBits(1);
    data.m_show_tooltips        = bs.GetBits(1);
    data.m_allow_profanity      = bs.GetBits(1);
    data.m_chat_balloons        = bs.GetBits(1);
    data.m_show_archetype       = (WindowState)bs.GetBits(3);
    data.m_show_supergroup      = (WindowState)bs.GetBits(3);
    data.m_show_player_name     = (WindowState)bs.GetBits(3);
    data.m_show_player_bars     = (WindowState)bs.GetBits(3);
    data.m_show_enemy_name      = (WindowState)bs.GetBits(3);
    data.m_show_enemy_bars      = (WindowState)bs.GetBits(3);
    data.m_show_player_reticles = (WindowState)bs.GetBits(3);
    data.m_show_enemy_reticles  = (WindowState)bs.GetBits(3);
    data.m_show_assist_reticles = (WindowState)bs.GetBits(3);
    data.m_chat_font_size       = bs.GetPackedBits(5);
}

void SaveClientOptions::serializeto(BitStream &tgt) const
{
    qDebug() << "Serializing to ClientOptions";
    tgt.StorePackedBits(1,65);
    tgt.StoreFloat(data.m_mouse_speed);
    tgt.StoreFloat(data.m_turn_speed);
    tgt.StoreBits(1,data.m_mouse_invert);
    tgt.StoreBits(1,data.m_fade_chat_wnd);
    tgt.StoreBits(1,data.m_fade_nav_wnd);
    tgt.StoreBits(1,data.m_show_tooltips);
    tgt.StoreBits(1,data.m_allow_profanity);
    tgt.StoreBits(1,data.m_chat_balloons);
    tgt.StoreBits(3,data.m_show_archetype);
    tgt.StoreBits(3,data.m_show_supergroup);
    tgt.StoreBits(3,data.m_show_player_name);
    tgt.StoreBits(3,data.m_show_player_bars);
    tgt.StoreBits(3,data.m_show_enemy_name);
    tgt.StoreBits(3,data.m_show_enemy_bars);
    tgt.StoreBits(3,data.m_show_player_reticles);
    tgt.StoreBits(3,data.m_show_enemy_reticles);
    tgt.StoreBits(3,data.m_show_assist_reticles);
    tgt.StoreBits(5,data.m_chat_font_size);
}
