#include "map_definitions.h"
#include "entitydata_definitions.h"

#include "Components/Logging.h"
#include <QtCore/QFileInfoList>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

const uint32_t DEFAULT_MAP_IDX = 24; // Outbreak

static std::vector<MapData> g_defined_map_datas =
{
    // City_Zones
    {24, "City_00_01", "Outbreak", MapType::CITY },
    {1, "City_01_01", "Atlas Park", MapType::CITY },
    {5, "City_01_02", "King's Row", MapType::CITY },
    {29, "City_01_03", "Galaxy City", MapType::CITY },
    {6, "City_02_01", "Steel Canyon", MapType::CITY },
    {7, "City_02_02", "Skyway City", MapType::CITY },
    {8, "City_03_01", "Talos Island", MapType::CITY },
    {9, "City_03_02", "Independence Port", MapType::CITY },
    {10, "City_04_01", "Founders' Falls", MapType::CITY },
    {11, "City_04_02", "Brickstown", MapType::CITY },
    {61, "City_05_01", "Peregrine Island", MapType::CITY },

    // Hazards
    {12, "Hazard_01_01", "Perez Park", MapType::HAZARD },
    {13, "Hazard_02_01", "Boomtown", MapType::HAZARD },
    {43, "Hazard_03_01", "Dark Astoria", MapType::HAZARD },
    {15, "Hazard_04_01", "Crey's Folly", MapType::HAZARD },
    {50000, "Hazard_04_02", "Enviro Nightmare", MapType::HAZARD },
    {9928, "Hazard_05_01", "Elysium", MapType::HAZARD },

    // Trials
    {18, "Trial_01_01", "Abandoned Sewer Network", MapType::TRIAL },
    {23, "Trial_01_02", "Sewer Network", MapType::TRIAL },
    {84, "Trial_02_01", "Faultline", MapType::TRIAL },
    {20, "Trial_03_01", "Terra Volta", MapType::TRIAL },
    {21, "Trial_04_01", "Eden", MapType::TRIAL },
    {22, "Trial_04_02", "The Hive", MapType::TRIAL },
    {60, "Trial_05_01", "Rikti Crash Site", MapType::TRIAL },

    // Missions
    {55000, "5th_Column", "5th_Column", MapType::MISSION },
    {55001, "Abandoned_Office", "Abandoned_Office", MapType::MISSION },
    {55002, "Abandoned_Warehouse", "Abandoned_Warehouse", MapType::MISSION },
    {55003, "Caves", "Caves", MapType::MISSION },
    {55004, "COT", "COT", MapType::MISSION },
    {55005, "Office", "Office", MapType::MISSION },
    {55006, "Outdoor_City", "Outdoor_City", MapType::MISSION },
    {55007, "Outdoor_Forest", "Outdoor_Forest", MapType::MISSION },
    {55008, "Outdoor_Industrial", "Outdoor_Industrial", MapType::MISSION },
    {55009, "Outdoor_Missions", "Outdoor_Missions", MapType::OUTDOOR_MISSION },
    {55010, "Outdoor_Ruined", "Outdoor_Ruined", MapType::MISSION },
    {55011, "Sewers", "Sewers", MapType::MISSION },
    {55012, "Tech", "Tech", MapType::MISSION },
    {55013, "unique", "unique", MapType::UNIQUE },
    {55014, "Warehouse", "Warehouse", MapType::MISSION }
};

QString getMissionPath(QString map_name, MissionCategory size)
{
    const MapData &map_data = getMapData(map_name);
    if (!map_data.m_mission_data.empty())
    {
        MissionMapData mission_data = map_data.m_mission_data.front();
        for (auto &data : map_data.m_mission_data)
        {
            if (data.m_mission_category == size)
            {
                mission_data = data;
            }
        }

        switch (map_data.m_map_type)
        {
            case MapType::MISSION:
            {
                if (mission_data.m_mission_category == MissionCategory::OUTDOOR)
                {
                    return QString("maps/Missions/%1/%2.txt").arg(map_name).arg(mission_data.m_layouts.front());
                }
                else
                {
                    return QString("maps/Missions/%1/%2/%3.txt").arg(map_name).arg(mission_data.m_mission_name).arg(mission_data.m_layouts.front());
                }
                break;
            }
            case MapType::UNIQUE:
            {
                return QString("maps/Missions/unique/%1/%2.txt").arg(mission_data.m_mission_name).arg(mission_data.m_layouts.front());
                break;
            }
            default:
            {
                break;
            }
        }
    }

    qWarning() << "Attempted to get mission filename for the map -- " << map_name << " -- That map doesn't have mission data loaded.";
    return QString();
}

void getMissionMapLevelData(QFileInfo map_level_folder, MapData &map_data)
{
    // maps/Missions/Sewers/Sewers_15/
    QString level = map_level_folder.fileName().mid(map_level_folder.fileName().lastIndexOf("_") + 1);
    QDir map_layout_dir(map_level_folder.filePath());
    map_layout_dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);

    MissionMapData mission_data;
    mission_data.m_mission_name = map_level_folder.baseName();
    qInfo() << "Mission name: " << mission_data.m_mission_name;

    if (mission_data.m_mission_name.contains("15"))
    {
        mission_data.m_mission_category = MissionCategory::TINY;
    }
    else if (mission_data.m_mission_name.contains("30"))
    {
        mission_data.m_mission_category = MissionCategory::SMALL;
    }
    else if (mission_data.m_mission_name.contains("45"))
    {
        mission_data.m_mission_category = MissionCategory::MEDIUM;
    }
    else if (mission_data.m_mission_name.contains("60"))
    {
        mission_data.m_mission_category = MissionCategory::LARGE;
    }
    else
    {
        mission_data.m_mission_category = MissionCategory::OUTDOOR;
    }

    qInfo() << "Map Size: " << mission_data.m_mission_category;

    for (auto &map_layout : map_layout_dir.entryInfoList())
    {
        if (map_data.m_map_type == MapType::MISSION && map_layout.isFile())
        {
            QString layout = map_layout.fileName().mid(0, map_layout.fileName().length() - 4);
            qInfo() << "Layout: " << layout;
            mission_data.m_layouts.push_back(layout);
        }
        else if (map_data.m_map_type == MapType::UNIQUE)
        {
            qInfo() << "Unique filename: " << map_layout.fileName();
            if (map_layout.isDir())
            {
                mission_data.m_mission_category = MissionCategory::TRIAL_ROOM;
                QDir trials(map_layout.filePath());
                trials.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);
                for (auto &trial : trials.entryInfoList())
                {
                    if (trial.isFile() && !trial.fileName().contains("beacon", Qt::CaseInsensitive) && !trial.fileName().contains("spawn", Qt::CaseInsensitive))
                    {
                        qInfo() << "Trial path: " << trial.filePath();
                        QString layout = trial.fileName().mid(0, trial.fileName().length() - 4);
                        qInfo() << "Layout: " << layout;
                        mission_data.m_layouts.push_back(layout);
                    }
                }
            }
            else
            {
                if (map_layout.isFile() && !map_layout.fileName().contains("trial"))
                {
                    if (map_layout.fileName().contains("Interdimensional"))
                    {
                        mission_data.m_mission_category = MissionCategory::INTERDIMENSIONAL;
                    }
                    else if (map_layout.fileName().contains("jumppuzzles"))
                    {
                        mission_data.m_mission_category = MissionCategory::JUMP_PUZZLE;
                    }

                    QString layout = map_layout.fileName().mid(0, map_layout.fileName().length() - 4);
                    qInfo() << "Layout: " << layout;
                    mission_data.m_layouts.push_back(layout);
                }
            }

        }
    }

    map_data.m_mission_data.push_back(mission_data);
}

void loadAllMissionMapData()
{
    for(auto& map_data : g_defined_map_datas)
    {
        qInfo() << "Loading mission data for: " << map_data.m_map_name;
        if (map_data.m_map_type == MapType::MISSION ||
            map_data.m_map_type == MapType::OUTDOOR_MISSION ||
            map_data.m_map_type == MapType::UNIQUE)
        {

            QString base_path = QFileInfo(QString("data/geobin/maps/Missions/%1").arg(QString(map_data.m_map_name))).filePath();
            QDir mapDir(base_path);
            mapDir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);
            if (!mapDir.exists())
            {
                qInfo() << "Failed to open map dir: " << mapDir.absolutePath();
            }
            if (mapDir.exists())
            {
                for (auto &map_level : mapDir.entryInfoList())
                {
                    if (map_level.isDir())
                    {
                        if (map_data.m_map_type == MapType::MISSION)
                        {
                            getMissionMapLevelData(map_level, map_data);
                        }
                        else if (map_data.m_map_type == MapType::UNIQUE)
                        {
                            getMissionMapLevelData(map_level, map_data);
                        }
                        else if(map_data.m_map_type == MapType::OUTDOOR_MISSION)
                        {
                            //getOutdoorUniqueMissionMapData(map_level, map_data);
                        }
                    }
                    else if (map_level.isFile())
                    {
                        // We're in an outdoor mission folder here, so we pass the parent folder.
                        getMissionMapLevelData(QFileInfo(mapDir.absolutePath()), map_data);
                    }
                }
            }
        }
    }
}

std::vector<MapData> &getAllMapData()
{
    return g_defined_map_datas;
}

MapData &getMapData(QString &map_name)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(map_name.contains(map_data.m_map_name, Qt::CaseInsensitive))
            return map_data;
    }

    // If no map is found, log a warning and return Outbreak's data.
    qWarning() << "No match for \"" << map_name << "\" in g_defined_map_datas."
               << "Returning Outbreak's map data as default...";
    return g_defined_map_datas[0];
}

uint32_t getMapIndex(const QString &map_name)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(map_name.contains(map_data.m_map_name, Qt::CaseInsensitive))
            return map_data.m_map_idx;
    }

    // log a warning because this part of the code is called when things went wrong
    qWarning() << "No matching \"" << map_name << "\" in g_defined_map_datas to sent map name."
               << "Returning Outbreak's map index as default...";

    // defaulting to Outbreak's map name
    return DEFAULT_MAP_IDX;
}

/// \note this functions returns the string by value, since m_display_map_name is QByteArray
QString getDisplayMapName(QString &map_name)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(map_name.contains(map_data.m_map_name, Qt::CaseInsensitive))
            return map_data.m_display_map_name;
    }

    // log a warning because this part of the code is called when things went wrong
    qWarning() << "No matching \"" << map_name << "\" in g_defined_map_datas to sent map name."
               << "Returning Outbreak's display map name as default...";

    // defaulting to Outbreak's map name
    return g_defined_map_datas[0].m_display_map_name;
}

QString getDisplayMapName(uint32_t index)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(index == map_data.m_map_idx)
            return map_data.m_display_map_name;
    }
    // Log a warning and return Outbreak if nothing found
    qWarning() << "Cannot find map index \"" << index << "\" ."
               << "Returning Outbreak's display map name as default...";
    return g_defined_map_datas[0].m_display_map_name;
}

QString getEntityDisplayMapName(const EntityData &ed)
{
    return getDisplayMapName(ed.m_map_idx);
}

bool isEntityOnMissionMap(EntityData &ed)
{
    QString mapName = getMapName(ed.m_map_idx);
    // Hazard and Trial maps are considered as mission maps
    return mapName.contains("Hazard") || mapName.contains("Trial");
}

QString getMapName(uint32_t index)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(index == map_data.m_map_idx)
            return map_data.m_map_name;
    }
    // Return Outbreak if nothing found
    qWarning() << "Cannot find map index \"" << index << "\" ."
               << "Returning Outbreak's map name as default...";
    return g_defined_map_datas[0].m_map_name;
}

QString getMapPath(uint32_t index)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(index == map_data.m_map_idx)
        {
            if (!map_data.m_mission_data.empty())
            {
                return getMissionPath(map_data.m_map_name, MissionCategory::MEDIUM);
            }
            return QString("maps/City_Zones/%1/%1.txt").arg(QString(map_data.m_map_name));
        }
    }
    qWarning() << "Cannot find map index \"" << index << "\" ."
               << "Returning Outbreak's map path as default...";
    return QString("maps/City_Zones/%1/%1.txt").arg(QString(g_defined_map_datas[0].m_map_name));
}

QString getMapPath(EntityData &ed)
{
    return getMapPath(ed.m_map_idx);
}

QString getMapPath(QString &map_name)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(map_name.contains(map_data.m_map_name, Qt::CaseInsensitive))
        {
            if (!map_data.m_mission_data.empty())
            {
                return getMissionPath(map_data.m_map_name, MissionCategory::MEDIUM);
            }
            return QString("maps/City_Zones/%1/%1.txt").arg(QString(map_data.m_map_name));
        }
    }

    // log a warning because this part of the code is called when things went wrong
    qWarning() << "No matching map path in g_defined_map_datas to sent map name."
               << "Returning Outbreak's display map path as default...";

    // defaulting to Outbreak's map name
    return QString("maps/City_Zones/%1/%1.txt").arg(QString(g_defined_map_datas[0].m_map_name));
}
