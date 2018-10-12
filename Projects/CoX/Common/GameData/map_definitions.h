/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <glm/vec2.hpp>
#include <QtCore/QString>
#include <vector>

struct EntityData;

struct Map_Data
{
    glm::vec2 Location;
    glm::vec2 TextLocation;
    QString Name;
    QString Icon;
};

struct MapData
{
    uint32_t m_map_idx;
    QString m_map_name;             // City_00_01, City_01_01, etc...
    QString m_map_path;             // The ones ending with .txt
    QString m_display_map_name;     // Outbreak, Atlas Park...
};
typedef std::vector<Map_Data> AllMaps_Data;

uint32_t       getMapIndex(const QString &map_name);
const std::vector<MapData> &getAllMapData();
const MapData &getMapData(const QString &map_name);
QString        getMapName(const QString &map_name);
QString        getMapName(size_t map_idx);
const QString &getDisplayMapName(size_t index);
QString        getEntityDisplayMapName(const EntityData &ed);
bool           isEntityOnMissionMap(const EntityData &ed);
QString        getMapPath(size_t index);
QString        getMapPath(const EntityData &ed);
