/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "SaveClientOptions.h"

void SaveClientOptions::serializefrom(BitStream & bs)
{
    qDebug() << "Serializing options from Client";
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

#define CLIENT_OPT(type,var)\
    ClientOption {#var,{{type,&var}} }

void ClientOptions::init()
{
    m_opts = {
        CLIENT_OPT(ClientOption::t_float,time),
        CLIENT_OPT(ClientOption::t_float,timescale),
        CLIENT_OPT(ClientOption::t_float,timestepscale),
        CLIENT_OPT(ClientOption::t_int,pause),
        CLIENT_OPT(ClientOption::t_int,disablegurneys),
        CLIENT_OPT(ClientOption::t_int,canlook),
        CLIENT_OPT(ClientOption::t_int,camrotate),
        CLIENT_OPT(ClientOption::t_int,forward),
        CLIENT_OPT(ClientOption::t_int,backward),
        CLIENT_OPT(ClientOption::t_int,left),
        CLIENT_OPT(ClientOption::t_int,right),
        CLIENT_OPT(ClientOption::t_int,up),
        CLIENT_OPT(ClientOption::t_int,down),
        CLIENT_OPT(ClientOption::t_int,nocoll),
        CLIENT_OPT(ClientOption::t_int,nosync),
        CLIENT_OPT(ClientOption::t_float,speed_turn),
        CLIENT_OPT(ClientOption::t_int,turnleft),
        CLIENT_OPT(ClientOption::t_int,turnright),
        CLIENT_OPT(ClientOption::t_int,zoomin),
        CLIENT_OPT(ClientOption::t_int,zoomout),
        CLIENT_OPT(ClientOption::t_int,lookup),
        CLIENT_OPT(ClientOption::t_int,lookdown),
        CLIENT_OPT(ClientOption::t_int,third),
        CLIENT_OPT(ClientOption::t_int,first),
        CLIENT_OPT(ClientOption::t_float,velscale),
        CLIENT_OPT(ClientOption::t_float,yaw),
        CLIENT_OPT(ClientOption::t_float,mouse_speed),
        CLIENT_OPT(ClientOption::t_int,mouse_invert),
        CLIENT_OPT(ClientOption::t_int,autorun),
        CLIENT_OPT(ClientOption::t_int,controldebug),
        CLIENT_OPT(ClientOption::t_int,nostrafe),
        CLIENT_OPT(ClientOption::t_int,alwaysmobile),
        CLIENT_OPT(ClientOption::t_int,repredict),
        CLIENT_OPT(ClientOption::t_int,neterrorcorrection),
        CLIENT_OPT(ClientOption::t_float,speed_scale),
        CLIENT_OPT(ClientOption::t_int,svr_lag),
        CLIENT_OPT(ClientOption::t_int,svr_lag_vary),
        CLIENT_OPT(ClientOption::t_int,svr_pl),
        CLIENT_OPT(ClientOption::t_int,svr_oo_packets),
        CLIENT_OPT(ClientOption::t_int,client_pos_id),
        CLIENT_OPT(ClientOption::t_int,atest0),
        CLIENT_OPT(ClientOption::t_int,atest1),
        CLIENT_OPT(ClientOption::t_int,atest2),
        CLIENT_OPT(ClientOption::t_int,atest3),
        CLIENT_OPT(ClientOption::t_int,atest4),
        CLIENT_OPT(ClientOption::t_int,atest5),
        CLIENT_OPT(ClientOption::t_int,atest6),
        CLIENT_OPT(ClientOption::t_int,atest7),
        CLIENT_OPT(ClientOption::t_int,atest8),
        CLIENT_OPT(ClientOption::t_int,atest9),
        CLIENT_OPT(ClientOption::t_int,predict),
        CLIENT_OPT(ClientOption::t_int,notimeout),
        CLIENT_OPT(ClientOption::t_int,selected_ent_server_index),
    };
}
#undef CLIENT_OPT

//! @}
