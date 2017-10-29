#pragma once

#include "Common/GameData/CoXHash.h"
#include "Common/GameData/costume_definitions.h"
#include "Common/GameData/origin_definitions.h"
#include "Common/GameData/charclass_definitions.h"
class ColorAndPartPacker;
class QString;
class MapServerData
{
        ColorAndPartPacker *packer_instance;
        bool            read_costumes(const QString &directory_path);
        bool            read_colors(const QString &src_filename);
        bool            read_origins(const QString &src_filename);
        bool            read_classes(const QString &src_filename);
public:
                        MapServerData();
                        ~MapServerData();
        bool            read_runtime_data(const QString &directory_path);
        ColorAndPartPacker *getPacker() { return packer_instance; }
        Pallette_Data   m_supergroup_colors;
        CostumeSet_Data m_costume_store;
        Parse_AllOrigins m_player_origins;
        Parse_AllOrigins m_other_origins;
        Parse_AllCharClasses m_player_classes;
        Parse_AllCharClasses m_other_classes;
};
