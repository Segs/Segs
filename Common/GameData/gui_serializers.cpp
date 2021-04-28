/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "gui_serializers.h"

#include "gui_definitions.h"
#include "DataStorage.h"
#include "Components/serialization_common.h"

CEREAL_CLASS_VERSION(GUISettings, GUISettings::class_version) // register GUISettings class version

template<class Archive>
void serialize(Archive &archive, GUISettings &gui, uint32_t const version)
{
    if(version != GUISettings::class_version)
    {
        qCritical() << "Failed to serialize GUISettings, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("TeamBuffs",gui.m_team_buffs));
    archive(cereal::make_nvp("ChatChannel",gui.m_cur_chat_channel));
    archive(cereal::make_nvp("PowersTray",gui.m_powers_tray_mode));
    archive(cereal::make_nvp("InspTray",gui.m_insps_tray_mode));
    archive(cereal::make_nvp("Tray1Page",gui.m_tray1_number));
    archive(cereal::make_nvp("Tray2Page",gui.m_tray2_number));
    archive(cereal::make_nvp("ChatTopFlags",gui.m_chat_top_flags));
    archive(cereal::make_nvp("ChatBottomFlags",gui.m_chat_bottom_flags));
    archive(cereal::make_nvp("ChatDividerPos",gui.m_chat_divider_pos));
    archive(cereal::make_nvp("Windows",gui.m_wnds));
}

void saveTo(const GUISettings &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"GUISettings",baseName,text_format);
}

SPECIALIZE_VERSIONED_SERIALIZATIONS(GUISettings)

void serializeToDb(const GUISettings &data, QString &tgt)
{
    std::ostringstream ostr;
    {
        cereal::JSONOutputArchive ar(ostr);
        ar(data);
    }
    tgt = QString::fromStdString(ostr.str());
}

void serializeFromDb(GUISettings &data,const QString &src)
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

//! @}
