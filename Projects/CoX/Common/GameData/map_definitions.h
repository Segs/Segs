/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <vector>

enum MapXferType : uint8_t
{
    DOOR        = 0,
    ZONE        = 1,
    ELEVATOR    = 2
};

enum MapType : uint8_t
{
    CITY                    = 0,
    TRIAL                   = 1,
    HAZARD                  = 2,
    MISSION                 = 3,
    OUTDOOR_MISSION         = 4,
    UNIQUE                  = 5
};

enum MissionCategory : uint8_t
{
    TINY                = 1,
    SMALL               = 2,
    MEDIUM              = 3,
    LARGE               = 4,
    OUTDOOR             = 5,
    INTERDIMENSIONAL    = 6,
    JUMP_PUZZLE         = 7,
    TRIAL_ROOM          = 8
};

struct EntityData;

struct Map_Data
{
    glm::vec2 Location;
    glm::vec2 TextLocation;
    QByteArray Name;
    QByteArray Icon;
};

struct MissionMapData
{
    QString m_mission_name;
    MissionCategory m_mission_category;
    std::vector<QString> m_layouts;
};


struct MapData
{
    MapData(uint32_t idx,QByteArray internal_name,QByteArray display_name,MapType kind) :
        m_map_idx(idx),m_map_name(internal_name),m_display_map_name(display_name),m_map_type(kind)
    {
    }

    uint32_t m_map_idx;
    QByteArray m_map_name;             // City_00_01, City_01_01, etc...
    QByteArray m_display_map_name;     // Outbreak, Atlas Park...
    MapType m_map_type;
    QByteArray m_map_path;             // The ones ending with .txt
    std::vector<MissionMapData> m_mission_data;
};

/*

MissionBase         /Category               /SubCategory                    /File

Sewers              /Sewers_15              /Sewers_15_Layout_01
Sewers              /Sewers_15              /Sewers_15_Layout_02_01

unique              /Interdimensional       /Interdimensional_01_01

unique              /TrialRooms             /Eden                           /Trial_01_01_room

Outdoor_Forest      /Outdoor_Forest_01

Outdoor_Missions    /Outdoor_Forest_01      /Outdoor_Forest_01a

Outdoor_Mission     /Outdoor_Unique         /Outdoor_Unique_Forest_01       /Outdoor_Unique_Forest_01c




unique: 
    maps/Missions/unique/Interdimensional/Interdimensional_01_01.bin
    maps/Missions/unique/TrialRooms/Eden/Trial_0X_0X_room.bin   //  with other Beacon, MissionBeacon, Spawns etc bins. has subfolder for audio..
    maps/Missions/unique/jumppuzzles/Jumppuzzles_Layout_01.bin  // ungrouped version

outdoor_missions
    Has a subfolder for each Outdoor_ mission map
    maps/Missions/Outdoor_Missions/Outdoor_Forest_01/Outdoor_Forest_01a.bin
    maps/Missions/Outdoor_Missions/Outdoor_Forest_01/Outdoor_Forest_01c.bin

    maps/Missions/Outdoor_Missions/Outdoor_Industrial_02/Outdoor_Industrial_02a.bin
    maps/Missions/Outdoor_Missions/Outdoor_Industrial_02/Outdoor_Industrial_02b.bin
    maps/Missions/Outdoor_Missions/Outdoor_Industrial_02/Outdoor_Industrial_02c.bin

    Has subfolder for Graveyard which doesn't seem to have a regular map

    Has subfolder for uniques, which again has subfolders for existing outdoor maps.
    maps/Missions/Outdoor_Missions/Outdoor_Unique/Outdoor_Unique_Forest_01/Outdoor_Unique_Forest_01c.bin
    maps/Missions/Outdoor_Missions/Outdoor_Unique/Outdoor_Unique_Forest_02/Outdoor_Unique_Forest_02c.bin
    maps/Missions/Outdoor_Missions/Outdoor_Unique/Outdoor_Unique_Ruined_01/Outdoor_Unique_Ruined_01a.bin
    maps/Missions/Outdoor_Missions/Outdoor_Unique/Outdoor_Unique_City_02/Outdoor_Unique_City_02a.bin
*/

struct MapXferData
{
    QString m_node_name;
    QString m_target_map_name;      // City_01_01
    QString m_target_spawn_name;    // Name of related spawnLocation node.
    glm::vec3 m_position;
    MapXferType m_transfer_type;
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_node_name, m_target_map_name, m_target_spawn_name, m_position, m_transfer_type);
    }
};

using AllMaps_Data = std::vector<Map_Data>;

uint32_t       getMapIndex(const QString &map_name);
std::vector<MapData> &getAllMapData();
MapData &getMapData(QString &map_name);
QString        getMapName(QString &map_name);
QString        getMapName(uint32_t map_idx);
QString        getDisplayMapName(uint32_t index);
QString        getEntityDisplayMapName(const EntityData &ed);
bool           isEntityOnMissionMap(EntityData &ed);
QString        getMapPath(uint32_t index);
QString        getMapPath(EntityData &ed);
QString        getMapPath(QString &map_name);

void           loadAllMissionMapData();
void           getMissionMapLevelData(QFileInfo map_level_folder, MapData &map_data);
void           getMissionMapLayoutData(QFileInfo maps_layout, uint8_t map_level, MapData& map_data);
QString        getMissionPath(QString map_name, MissionCategory size);
uint8_t        getMissionMapLevelRange(uint8_t char_level);
void           getMissionMapOutdoorData(QFileInfo map_file, MapData& map_data);
