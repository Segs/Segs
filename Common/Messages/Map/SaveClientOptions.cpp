/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "SaveClientOptions.h"

#include "Components/BitStream.h"

using namespace SEGSEvents;

void SaveClientOptions::serializefrom(BitStream & bs)
{
    qCDebug(logSettings) << "Serializing options from Client";
    data.m_mouse_speed          = bs.GetFloat();
    data.m_turn_speed           = bs.GetFloat();
    data.m_mouse_invert         = bs.GetBits(1);
    data.m_fade_chat_wnd        = bs.GetBits(1);
    data.m_fade_nav_wnd         = bs.GetBits(1);
    data.m_show_tooltips        = bs.GetBits(1);
    data.m_allow_profanity      = bs.GetBits(1);
    data.m_chat_balloons        = bs.GetBits(1);
    data.m_show_archetype       = (ReticleVisibility)bs.GetBits(3);
    data.m_show_supergroup      = (ReticleVisibility)bs.GetBits(3);
    data.m_show_player_name     = (ReticleVisibility)bs.GetBits(3);
    data.m_show_player_bars     = (ReticleVisibility)bs.GetBits(3);
    data.m_show_enemy_name      = (ReticleVisibility)bs.GetBits(3);
    data.m_show_enemy_bars      = (ReticleVisibility)bs.GetBits(3);
    data.m_show_player_reticles = (ReticleVisibility)bs.GetBits(3);
    data.m_show_enemy_reticles  = (ReticleVisibility)bs.GetBits(3);
    data.m_show_assist_reticles = (ReticleVisibility)bs.GetBits(3);
    data.m_chat_font_size       = bs.GetPackedBits(5);
}

void SaveClientOptions::serializeto(BitStream &tgt) const
{
    qCDebug(logSettings) << "Serializing to ClientOptions";
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

//! @}
