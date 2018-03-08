/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "clientoptions_serializers.h"

#include "DataStorage.h"
#include "Servers/MapServer/Events/ClientOptions.h"
#include "serialization_common.h"

CEREAL_CLASS_VERSION(ClientOptions, 1); // register Client Options class version

template<class Archive>
void serialize(Archive &archive, ClientOptions &co, uint32_t const version)
{
    archive(cereal::make_nvp("MouseSpeed",co.m_mouse_speed));
    archive(cereal::make_nvp("TurnSpeed",co.m_turn_speed));
    archive(cereal::make_nvp("InvertMouse",co.m_mouse_invert));
    archive(cereal::make_nvp("FadeChatWindow",co.m_fade_chat_wnd));
    archive(cereal::make_nvp("FadeNavWindow",co.m_fade_nav_wnd));
    archive(cereal::make_nvp("ShowTooltips",co.m_show_tooltips));
    archive(cereal::make_nvp("AllowProfanity",co.m_allow_profanity));
    archive(cereal::make_nvp("ChatBalloons",co.m_chat_balloons));
    archive(cereal::make_nvp("ShowArchetype",co.m_show_archetype));
    archive(cereal::make_nvp("ShowSuperGroup",co.m_show_supergroup));
    archive(cereal::make_nvp("PlayerName",co.m_show_player_name));
    archive(cereal::make_nvp("PlayerBars",co.m_show_player_bars));
    archive(cereal::make_nvp("EnemyName",co.m_show_enemy_name));
    archive(cereal::make_nvp("EnemyBars",co.m_show_enemy_bars));
    archive(cereal::make_nvp("PlayerReticles",co.m_show_player_reticles));
    archive(cereal::make_nvp("EnemyReticles",co.m_show_enemy_reticles));
    archive(cereal::make_nvp("AssistReticles",co.m_show_assist_reticles));
    archive(cereal::make_nvp("FontSize",co.m_chat_font_size));
}

void saveTo(const ClientOptions &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"ClientOptions",baseName,text_format);
}

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &archive, ClientOptions &co, uint32_t const version);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &archive, ClientOptions &co, uint32_t const version);

void serializeToDb(const ClientOptions &data, QString &tgt)
{
    std::ostringstream ostr;
    {
        cereal::JSONOutputArchive ar(ostr);
        ar(data);
    }
    tgt = QString::fromStdString(ostr.str());
}

void serializeFromDb(ClientOptions &data,const QString &src)
{
    if(src.isEmpty())
        return;
    std::istringstream istr;
    istr.str(src.toStdString());
    {
        cereal::JSONInputArchive ar(istr);
        ar(data);
    }
}
