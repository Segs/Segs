#include "map_definitions.h"
#include "entitydata_definitions.h"

#include "Utils/string_utils.h"
#include "Components/Logging.h"
#include "Common/Utils/IServiceLocator.h"

const uint32_t DEFAULT_MAP_IDX = 24; // Outbreak

static Vector<MapData> g_defined_map_datas =
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

String getMissionPath(StringView map_name, MissionCategory size)
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
                    return StringUtils::fmt("maps/Missions/%.*s/%s.txt",(int)map_name.size(),map_name.data(),mission_data.m_layouts.front().c_str());
                }
                else
                {
                    return StringUtils::fmt("maps/Missions/%.*s/%s/%s.txt",(int)map_name.size(),map_name.data(),mission_data.m_mission_name.c_str(),mission_data.m_layouts.front().c_str());
                }
                break;
            }
            case MapType::UNIQUE:
            {
                return StringUtils::fmt("maps/Missions/unique/%s/%s.txt",mission_data.m_mission_name.c_str(),mission_data.m_layouts.front().c_str());
            }
            default:
            {
                break;
            }
        }
    }

    sWarning() << "Attempted to get mission filename for the map -- " << map_name << " -- That map doesn't have mission data loaded.";
    return String();
}

void getMissionMapLevelData(StringView map_level_folder, MapData &map_data)
{
    // maps/Missions/Sewers/Sewers_15/
    StringView fname=PathUtils::get_file(map_level_folder);
    StringView level = fname.substr(fname.find_last_of('_') + 1,-1);
    //QDir map_layout_dir(map_level_folder.filePath());
    //map_layout_dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);

    MissionMapData mission_data;
    mission_data.m_mission_name = PathUtils::get_basename(map_level_folder);
    sInfo() << "Mission name: " << mission_data.m_mission_name;

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

    sInfo() << "Map Size: " << mission_data.m_mission_category;

    //std::filesystem::directory_iterator end_iter;
    auto fs = SEGS::getServiceLocator()->getFS();
    fs->visitEntries(map_level_folder, [&](StringView entry, bool is_dir)-> auto {
        auto entry_name = PathUtils::get_file(entry);
        if (map_data.m_map_type == MapType::MISSION && !is_dir) //std::filesystem::is_regular_file(dir_iter->status())
        {
            sInfo() << "Layout: " << PathUtils::get_file(entry);
            mission_data.m_layouts.emplace_back(entry);
        }
        else if (map_data.m_map_type == MapType::UNIQUE)
        {
            sInfo() << "Unique filename: " << PathUtils::get_file(entry);
            if (is_dir)
            {
                mission_data.m_mission_category = MissionCategory::TRIAL_ROOM;
                fs->visitEntries(entry, [&](StringView ptrial, bool is_dir)-> auto {
                    auto trial_entry_name = PathUtils::get_file(ptrial);
                    if (!is_dir && !StringUtils::contains(ptrial, "beacon") && StringUtils::contains(ptrial, "spawn"))
                    {
                        sInfo() << "Trial path: " << ptrial;
                        sInfo() << "Layout: " << PathUtils::get_basename(trial_entry_name);
                        mission_data.m_layouts.emplace_back(PathUtils::get_basename(trial_entry_name));
                    }
                    return SEGS::IFilesystem::VisitNext;
                });
            }
            else
            {
                if (!entry_name.contains("trial")) //std::filesystem::is_regular_file(dir_iter->status()) &&
                {
                    if (entry_name.contains("Interdimensional"))
                    {
                        mission_data.m_mission_category = MissionCategory::INTERDIMENSIONAL;
                    }
                    else if (entry_name.contains("jumppuzzles"))
                    {
                        mission_data.m_mission_category = MissionCategory::JUMP_PUZZLE;
                    }

                    StringView layout = entry_name.substr(0, entry_name.length() - 4);
                    sInfo() << "Layout: " << layout;
                    mission_data.m_layouts.emplace_back(layout);
                }
            }
        }
        return SEGS::IFilesystem::VisitNext;
    });

    map_data.m_mission_data.push_back(mission_data);
}

void loadAllMissionMapData()
{
    auto fs = SEGS::getServiceLocator()->getFS();
    for(auto& map_data : g_defined_map_datas)
    {
        sInfo() << "Loading mission data for: " << map_data.m_map_name;
        if (map_data.m_map_type != MapType::MISSION && map_data.m_map_type != MapType::OUTDOOR_MISSION &&
            map_data.m_map_type != MapType::UNIQUE)
            continue;

        String base_path = StringUtils::fmt("data/geobin/maps/Missions/%s",map_data.m_map_name.c_str());
        if(!fs->exists(base_path))
            {
            sInfo() << "Failed to open map dir: " << base_path;
            continue;
            }
        fs->visitEntries(base_path,[&](StringView p, bool is_dir)->auto {
            StringView fpath(p);
            if (!is_dir)
            {
                sInfo() << "Layout: " << p;
                // We're in an outdoor mission folder here, so we pass the parent folder.
                getMissionMapLevelData(fpath, map_data);
            }
            if (is_dir)
                {
                // skip the . and .. directories
                if (fpath == "." || fpath == "..")
                    return SEGS::IFilesystem::VisitNext;
                        if (map_data.m_map_type == MapType::MISSION)
                        {
                    getMissionMapLevelData(fpath, map_data);
                        }
                        else if (map_data.m_map_type == MapType::UNIQUE)
                        {
                    getMissionMapLevelData(fpath, map_data);
                        }
                        else if(map_data.m_map_type == MapType::OUTDOOR_MISSION)
                        {
                            //getOutdoorUniqueMissionMapData(map_level, map_data);
                        }
                    }
            return SEGS::IFilesystem::VisitNext;
        });
    }
}

Vector<MapData> &getAllMapData()
{
    return g_defined_map_datas;
}

MapData &getMapData(StringView map_name)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(StringUtils::contains(map_name,map_data.m_map_name, StringUtils::CaseInsensitive))
            return map_data;
    }

    // If no map is found, log a warning and return Outbreak's data.
    sWarning() << "No match for \"" << map_name << "\" in g_defined_map_datas."
               << "Returning Outbreak's map data as default...";
    return g_defined_map_datas[0];
}

uint32_t getMapIndex(const String &map_name)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(StringUtils::contains(map_name,map_data.m_map_name, StringUtils::CaseInsensitive))
            return map_data.m_map_idx;
    }

    // log a warning because this part of the code is called when things went wrong
    sWarning() << "No matching \"" << map_name << "\" in g_defined_map_datas to sent map name."
               << "Returning Outbreak's map index as default...";

    // defaulting to Outbreak's map name
    return DEFAULT_MAP_IDX;
}

/// \note this functions returns the string by value, since m_display_map_name is QByteArray
String getDisplayMapName(String &map_name)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(StringUtils::contains(map_name,map_data.m_map_name, StringUtils::CaseInsensitive))
            return map_data.m_display_map_name;
    }

    // log a warning because this part of the code is called when things went wrong
    sWarning() << "No matching \"" << map_name << "\" in g_defined_map_datas to sent map name."
               << "Returning Outbreak's display map name as default...";

    // defaulting to Outbreak's map name
    return g_defined_map_datas[0].m_display_map_name;
}

String getDisplayMapName(uint32_t index)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(index == map_data.m_map_idx)
            return map_data.m_display_map_name;
    }
    // Log a warning and return Outbreak if nothing found
    sWarning() << "Cannot find map index \"" << index << "\" ."
               << "Returning Outbreak's display map name as default...";
    return g_defined_map_datas[0].m_display_map_name;
}

String getEntityDisplayMapName(const EntityData &ed)
{
    return getDisplayMapName(ed.m_map_idx);
}

bool isEntityOnMissionMap(EntityData &ed)
{
    String mapName = getMapName(ed.m_map_idx);
    // Hazard and Trial maps are considered as mission maps
    return mapName.contains("Hazard") || mapName.contains("Trial");
}

String getMapName(uint32_t index)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(index == map_data.m_map_idx)
            return map_data.m_map_name;
    }
    // Return Outbreak if nothing found
    sWarning() << "Cannot find map index \"" << index << "\" ."
               << "Returning Outbreak's map name as default...";
    return g_defined_map_datas[0].m_map_name;
}

String getMapPath(uint32_t index)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(index == map_data.m_map_idx)
        {
            if (!map_data.m_mission_data.empty())
            {
                return getMissionPath(map_data.m_map_name, MissionCategory::MEDIUM);
            }
            return StringUtils::fmt("maps/City_Zones/%s/%s.txt",map_data.m_map_name.c_str(),map_data.m_map_name.c_str());
        }
    }
    sWarning() << "Cannot find map index \"" << index << "\" ."
               << "Returning Outbreak's map path as default...";
    return StringUtils::fmt("maps/City_Zones/%s/%s.txt",g_defined_map_datas[0].m_map_name.c_str(),g_defined_map_datas[0].m_map_name.c_str());
}

String getMapPath(EntityData &ed)
{
    return getMapPath(ed.m_map_idx);
}

String getMapPath(String &map_name)
{
    for (auto &map_data : g_defined_map_datas)
    {
        if(StringUtils::contains(map_name,map_data.m_map_name, StringUtils::CaseInsensitive))
        {
            if (!map_data.m_mission_data.empty())
            {
                return getMissionPath(map_data.m_map_name, MissionCategory::MEDIUM);
            }
            return StringUtils::fmt("maps/City_Zones/%s/%s.txt",map_data.m_map_name.c_str(),map_data.m_map_name.c_str());
        }
    }

    // log a warning because this part of the code is called when things went wrong
    sWarning() << "No matching map path in g_defined_map_datas to sent map name."
               << "Returning Outbreak's display map path as default...";

    // defaulting to 0th map - Outbreak's map name
    return StringUtils::fmt("maps/City_Zones/%s/%s.txt",g_defined_map_datas[0].m_map_name.c_str(),g_defined_map_datas[0].m_map_name.c_str());
}
