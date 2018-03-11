/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "gui_serializers.h"

#include "gui_definitions.h"
#include "DataStorage.h"
#include "serialization_common.h"

CEREAL_CLASS_VERSION(GUIWindow, 1); // register GUIWindow class version
CEREAL_CLASS_VERSION(GUISettings, 1); // register GUISettings class version

template<class Archive>
void serialize(Archive &archive, GUIWindow &wnd, uint32_t const version)
{
    archive(cereal::make_nvp("IDX",wnd.m_idx));
    archive(cereal::make_nvp("Mode",wnd.m_mode));
    archive(cereal::make_nvp("DraggableFrame",wnd.m_draggable_frame));
    archive(cereal::make_nvp("PosX",wnd.m_posx));
    archive(cereal::make_nvp("PosY",wnd.m_posx));
    archive(cereal::make_nvp("Width",wnd.m_width));
    archive(cereal::make_nvp("Height",wnd.m_height));
    archive(cereal::make_nvp("Locked",wnd.m_locked));
    archive(cereal::make_nvp("Color",wnd.m_color));
    archive(cereal::make_nvp("Alpha",wnd.m_alpha));
}

template<class Archive>
void serialize(Archive &archive, GUISettings &gui, uint32_t const version)
{
    archive(cereal::make_nvp("TeamBuffs",gui.m_team_buffs));
    archive(cereal::make_nvp("Windows",gui.m_wnds));
}

void saveTo(const GUISettings &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"GUISettings",baseName,text_format);
}

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &archive, GUISettings &co, uint32_t const version);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &archive, GUISettings &co, uint32_t const version);

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
