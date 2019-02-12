/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
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
    CITY        = 0,
    TRIAL       = 1,
    HAZARD      = 2,
    MISSION     = 3
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
    uint8_t m_level;        // Sewers_15
    uint8_t m_layout;       // Sewers_15_Layout_01
    uint8_t m_sub_layout;   // Sewers_15_Layout_01_01
    bool m_has_sub_layout;
};

struct MapData
{
    uint32_t m_map_idx;
    QByteArray m_map_name;             // City_00_01, City_01_01, etc...
    QByteArray m_display_map_name;     // Outbreak, Atlas Park...
    MapType m_map_type;
    QByteArray m_map_path;             // The ones ending with .txt
    std::vector<MissionMapData> m_mission_data;
};

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

QString        createMapPath(MapData &map_data);
void           loadAllMissionMapData();
void           getMissionMapLevelData(QFileInfo map_level_folder, MapData &map_data);
void           getMissionMapLayoutData(QFileInfo maps_layout, uint8_t map_level, MapData& map_data);
