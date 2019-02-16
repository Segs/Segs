#include "map_definitions.h"
#include "entitydata_definitions.h"

#include "Logging.h"

const uint32_t DEFAULT_MAP_IDX = 24; // Outbreak

static const std::vector<MapData> g_defined_map_datas =
{
    // City_Zones
    {24, "City_00_01", "maps/City_Zones/City_00_01/City_00_01.txt", "Outbreak"},
    {1, "City_01_01", "maps/City_Zones/City_01_01/City_01_01.txt", "Atlas Park"},
    {5, "City_01_02", "maps/City_Zones/City_01_02/City_01_02.txt", "King's Row"},
    {29, "City_01_03", "maps/City_Zones/City_01_03/City_01_03.txt", "Galaxy City"},
    {6, "City_02_01", "maps/City_Zones/City_02_01/City_02_01.txt", "Steel Canyon"},
    {7, "City_02_02", "maps/City_Zones/City_02_02/City_02_02.txt", "Skyway City"},
    {8, "City_03_01", "maps/City_Zones/City_03_01/City_03_01.txt", "Talos Island"},
    {9, "City_03_02", "maps/City_Zones/City_03_02/City_03_02.txt", "Independence Port"},
    {10, "City_04_01", "maps/City_Zones/City_04_01/City_04_01.txt", "Founders' Falls"},
    {11, "City_04_02", "maps/City_Zones/City_04_02/City_04_02.txt", "Brickstown"},
    {61, "City_05_01", "maps/City_Zones/City_05_01/City_05_01.txt", "Peregrine Island"},

    // Hazards
    {12, "Hazard_01_01", "maps/City_Zones/Hazard_01_01/Hazard_01_01.txt", "Perez Park"},
    {13, "Hazard_02_01", "maps/City_Zones/Hazard_02_01/Hazard_02_01.txt", "Boomtown"},
    {43, "Hazard_03_01", "maps/City_Zones/Hazard_03_01/Hazard_03_01.txt", "Dark Astoria"},
    {15, "Hazard_04_01", "maps/City_Zones/Hazard_04_01/Hazard_04_01.txt", "Crey's Folly"},
    {50000, "Hazard_04_02", "maps/City_Zones/Hazard_04_02/Hazard_04_02.txt", "Enviro Nightmare"},
    {9928, "Hazard_05_01", "maps/City_Zones/Hazard_05_01/Hazard_05_01.txt", "Elysium"},

    // Trials
    {18, "Trial_01_01", "maps/City_Zones/Trial_01_01/Trial_01_01.txt", "Abandoned Sewer Network"},
    {23, "Trial_01_02", "maps/City_Zones/Trial_01_02/Trial_01_02.txt", "Sewer Network"},
    {84, "Trial_02_01", "maps/City_Zones/Trial_02_01/Trial_02_01.txt", "Faultline"},
    {20, "Trial_03_01", "maps/City_Zones/Trial_03_01/Trial_03_01.txt", "Terra Volta"},
    {21, "Trial_04_01", "maps/City_Zones/Trial_04_01/Trial_04_01.txt", "Eden"},
    {22, "Trial_04_02", "maps/City_Zones/Trial_04_02/Trial_04_02.txt", "The Hive"},
    {60, "Trial_05_01", "maps/City_Zones/Trial_05_01/Trial_05_01.txt", "Rikti Crash Site"}
};

const std::vector<MapData> &getAllMapData()
{
    return g_defined_map_datas;
}

const MapData &getMapData(const QString &map_name)
{
    for (const auto &map_data : g_defined_map_datas)
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
    for (const auto &map_data : g_defined_map_datas)
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
QString getDisplayMapName(const QString &map_name)
{
    for (const auto &map_data : g_defined_map_datas)
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
    for (const auto &map_data : g_defined_map_datas)
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

bool isEntityOnMissionMap(const EntityData &ed)
{
    QString mapName = getMapName(ed.m_map_idx);
    // Hazard and Trial maps are considered as mission maps
    return mapName.contains("Hazard") || mapName.contains("Trial");
}

QString getMapName(uint32_t index)
{
    for (const auto &map_data : g_defined_map_datas)
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
    for (const auto &map_data : g_defined_map_datas)
    {
        if(index == map_data.m_map_idx)
            return map_data.m_map_path;
    }
    qWarning() << "Cannot find map index \"" << index << "\" ."
               << "Returning Outbreak's map path as default...";
    return g_defined_map_datas[0].m_map_path;
}

QString getMapPath(const EntityData &ed)
{
    return getMapPath(ed.m_map_idx);
}

QString getMapPath(const QString &map_name)
{
    for (const auto &map_data : g_defined_map_datas)
    {
        if(map_name.contains(map_data.m_map_name, Qt::CaseInsensitive))
            return map_data.m_map_path;
    }

    // log a warning because this part of the code is called when things went wrong
    qWarning() << "No matching map path in g_defined_map_datas to sent map name."
               << "Returning Outbreak's display map path as default...";

    // defaulting to Outbreak's map name
    return g_defined_map_datas[0].m_map_path;
}
