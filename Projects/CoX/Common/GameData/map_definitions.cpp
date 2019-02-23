#include "map_definitions.h"
#include "entitydata_definitions.h"

#include "Logging.h"
#include <qt5/QtCore/QFileInfoList>
#include <qt5/QtCore/QDir>
#include <qt5/QtCore/QFileInfo>

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
    {55006, "Outdoor_City", "Outdoor_City", MapType::MISSION_OUTDOOR },
    {55007, "Outdoor_Forest", "Outdoor_Forest", MapType::MISSION_OUTDOOR },
    {55008, "Outdoor_Industrial", "Outdoor_Industrial", MapType::MISSION_OUTDOOR },
    //{55009, "Outdoor_Missions", "Outdoor_Missions", MapType::MISSION_OUTDOOR },
    {55010, "Outdoor_Ruined", "Outdoor_Ruined", MapType::MISSION_OUTDOOR },
    {55011, "Sewers", "Sewers", MapType::MISSION },
    {55012, "Tech", "Tech", MapType::MISSION },
    //{55013, "unique", "unique", MapType::MISSION_UNIQUE },
    {55014, "Warehouse", "Warehouse", MapType::MISSION }
};

uint8_t getMissionMapLevelRange(uint8_t char_level)
{
    for (auto &level : level_ranges)
    {
        if (char_level >= level)
        {
            return level;
        }
    }

    return level_ranges[3]; // return 15 if a level isn't found.
}

QString getMissionPath(QString map_name, uint8_t char_level)
{
    MapData map_data = getMapData(map_name);
    uint8_t level_range = getMissionMapLevelRange(char_level);
    if (!map_data.m_mission_data.empty())
    {
        switch (map_data.m_map_type)
        {
            case MapType::MISSION:
            {
                MissionMapData mission_data;
                if (map_data.m_mission_data.contains(level_range))
                {
                    mission_data = map_data.m_mission_data[level_range];
                }
                else
                {
                    mission_data = map_data.m_mission_data.values().front();
                }
                
                MissionLayout layout = mission_data.m_layouts.front();
                if (layout.m_has_sub_layout)
                {
                    return QString("maps/Missions/%1/%1_%2/%1_%2_Layout_%3_%4.txt").arg(QString(map_name)).arg(mission_data.m_level).arg(layout.m_layout, 2, 10, QChar('0')).arg(layout.m_sub_layout, 2, 10, QChar('0'));
                }
                else
                {
                    return QString("maps/Missions/%1/%1_%2/%1_%2_Layout_%3.txt").arg(QString(map_name)).arg(mission_data.m_level).arg(layout.m_layout, 2, 10,QChar('0'));
                }                
                break;
            }
            case MapType::MISSION_OUTDOOR:
            {
                if (map_data.m_mission_data.contains(OUTDOOR_MISSION_LEVEL_RANGE))
                {
                    MissionMapData mission_data = map_data.m_mission_data[OUTDOOR_MISSION_LEVEL_RANGE];
                    MissionLayout layout = mission_data.m_layouts.front();
                    return QString("maps/Missions/%1/%1_%2.txt").arg(QString(map_name)).arg(layout.m_layout, 2, 10, QChar('0'));
                }
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
    for (auto &map_layout : map_layout_dir.entryInfoList())
    {
        if (map_data.m_map_type == MapType::MISSION)
        {
            getMissionMapLayoutData(map_layout, level.toInt(), map_data);
        }
        else if (map_data.m_map_type == MapType::MISSION_UNIQUE)
        {
            //TODO: Handle unique maps.
        }
    }
}
void getMissionMapLayoutData(QFileInfo map_layout, uint8_t map_level, MapData& map_data)
{
    if (map_layout.isFile())
    {
        // String parsing to get the layouts and sublayouts from Sewers_15_Layout_01_02
        QString layoutSubstring = map_layout.baseName().mid(map_layout.baseName().indexOf("Layout"));
        int layoutUnderscore = layoutSubstring.indexOf("_") + 1;
        int subLayoutUnderscore = layoutSubstring.lastIndexOf("_") + 1;
        QString sublayout_value = layoutSubstring.mid(subLayoutUnderscore);
        QString layout_value = layoutSubstring.mid(layoutUnderscore, 2);        

        if (!map_data.m_mission_data.contains(map_level))
        {
            MissionMapData mission_data;
            mission_data.m_level = map_level;
            map_data.m_mission_data[map_level] = mission_data;
        }

        MissionLayout layout;
        layout.m_layout = layout_value.toInt();
        if (layoutUnderscore != subLayoutUnderscore)
        {
            layout.m_sub_layout = sublayout_value.toInt();
            layout.m_has_sub_layout = true;
        }
        else
        {
            layout.m_has_sub_layout = false;
        }
        
        qInfo() << "Added mission layout data for: " << map_data.m_map_name;
        map_data.m_mission_data[map_level].m_layouts.push_back(layout);
    }
}

void getMissionMapOutdoorData(QFileInfo map_file, MapData& map_data)
{
    if (!map_data.m_mission_data.contains(OUTDOOR_MISSION_LEVEL_RANGE))
    {
        MissionMapData mission_data;
        mission_data.m_level = OUTDOOR_MISSION_LEVEL_RANGE;
        map_data.m_mission_data[OUTDOOR_MISSION_LEVEL_RANGE] = mission_data;
    }
    
    MissionLayout layout;
    layout.m_has_sub_layout = false;
    QString layoutString = map_file.baseName().mid(map_data.m_map_name.length() + 1, 2);
    layout.m_layout = layoutString.toInt();
    map_data.m_mission_data[OUTDOOR_MISSION_LEVEL_RANGE].m_layouts.push_back(layout);
}


void loadAllMissionMapData()
{
    for(auto& map_data : g_defined_map_datas)
    {
        qInfo() << "Loading mission data for: " << map_data.m_map_name;
        if (map_data.m_map_type == MapType::MISSION || 
            map_data.m_map_type == MapType::MISSION_OUTDOOR || 
            map_data.m_map_type == MapType::MISSION_UNIQUE)
        {
           
            QString base_path = QFileInfo(QString("data/geobin/maps/Missions/%1").arg(QString(map_data.m_map_name))).filePath();
            qInfo() << "Mission map for path: " << base_path;
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
                        else if (map_data.m_map_type == MapType::MISSION_UNIQUE)
                        {
                            //TODO: Handle unique maps
                        }
                    }
                    else if (map_level.isFile())
                    {
                        getMissionMapOutdoorData(map_level, map_data);
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

QString getMapPath(uint32_t index, uint8_t char_level)
{    
    for (auto &map_data : g_defined_map_datas)
    {
        if(index == map_data.m_map_idx)
        {
            if (!map_data.m_mission_data.empty())
            {
                return getMissionPath(map_data.m_map_name, char_level);
            }
            return QString("maps/City_Zones/%1/%1.txt").arg(QString(map_data.m_map_name));
        }
    }
    qWarning() << "Cannot find map index \"" << index << "\" ."
               << "Returning Outbreak's map path as default...";
    return QString("maps/City_Zones/%1/%1.txt").arg(QString(g_defined_map_datas[0].m_map_name));
}

QString getMapPath(EntityData &ed, uint8_t char_level)
{
    return getMapPath(ed.m_map_idx, char_level);
}

QString getMapPath(QString &map_name, uint8_t char_level)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(map_name.contains(map_data.m_map_name, Qt::CaseInsensitive))
        {
            if (!map_data.m_mission_data.empty())
            {
                return getMissionPath(map_data.m_map_name, char_level);
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
