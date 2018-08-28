/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "clientoptions_serializers.h"

#include "clientoptions_definitions.h"
#include "DataStorage.h"
#include "serialization_common.h"

const constexpr uint32_t ClientOptions::class_version;
// register Client Options class version
CEREAL_CLASS_VERSION(ClientOptions, ClientOptions::class_version)

template<class Archive>
void serialize(Archive &archive, ClientOptions &co, uint32_t const version)
{
    if(version != ClientOptions::class_version)
    {
        qCritical() << "Failed to serialize ClientOptions, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("FirstPersonView",co.m_first_person_view));
    archive(cereal::make_nvp("MouseSpeed",co.m_mouse_speed));
    archive(cereal::make_nvp("TurnSpeed",co.m_turn_speed));
    archive(cereal::make_nvp("InvertMouse",co.m_mouse_invert));
    archive(cereal::make_nvp("FadeChatWindow",co.m_fade_chat_wnd));
    archive(cereal::make_nvp("FadeNavWindow",co.m_fade_nav_wnd));
    archive(cereal::make_nvp("ShowTooltips",co.m_show_tooltips));
    archive(cereal::make_nvp("AllowProfanity",co.m_allow_profanity));
    archive(cereal::make_nvp("ChatBalloons",co.m_chat_balloons));
    archive(cereal::make_nvp("ShowArchetype",(uint32_t)co.m_show_archetype));
    archive(cereal::make_nvp("ShowSuperGroup",(uint32_t)co.m_show_supergroup));
    archive(cereal::make_nvp("PlayerName",(uint32_t)co.m_show_player_name));
    archive(cereal::make_nvp("PlayerBars",(uint32_t)co.m_show_player_bars));
    archive(cereal::make_nvp("EnemyName",(uint32_t)co.m_show_enemy_name));
    archive(cereal::make_nvp("EnemyBars",(uint32_t)co.m_show_enemy_bars));
    archive(cereal::make_nvp("PlayerReticles",(uint32_t)co.m_show_player_reticles));
    archive(cereal::make_nvp("EnemyReticles",(uint32_t)co.m_show_enemy_reticles));
    archive(cereal::make_nvp("AssistReticles",(uint32_t)co.m_show_assist_reticles));
    archive(cereal::make_nvp("FontSize",(uint32_t)co.m_chat_font_size));
}

void saveTo(const ClientOptions &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"ClientOptions",baseName,text_format);
}

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &archive, ClientOptions &co, uint32_t const version);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &archive, ClientOptions &co, uint32_t const version);

//! @}
