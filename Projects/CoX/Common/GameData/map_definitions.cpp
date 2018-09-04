#include "map_definitions.h"

#include "Logging.h"

static const std::vector<MapData> g_defined_map_datas =
{
    // City_Zones
    {0, "City_00_01", "maps/City_Zones/City_00_01/City_00_01.txt", "Outbreak"},
    {1, "City_01_01", "maps/City_Zones/City_01_01/City_01_01.txt", "Atlas Park"},
    {2, "City_01_02", "maps/City_Zones/City_01_02/City_01_02.txt", "King's Row"},
    {3, "City_01_03", "maps/City_Zones/City_01_03/City_01_03.txt", "Galaxy City"},
    {4, "City_02_01", "maps/City_Zones/City_02_01/City_02_01.txt", "Steel Canyon"},
    {5, "City_02_02", "maps/City_Zones/City_02_02/City_02_02.txt", "Skyway City"},
    {6, "City_03_01", "maps/City_Zones/City_03_01/City_03_01.txt", "Talos Island"},
    {7, "City_03_02", "maps/City_Zones/City_03_02/City_03_02.txt", "Independence Port"},
    {8, "City_04_01", "maps/City_Zones/City_04_01/City_04_01.txt", "Founders' Falls"},
    {9, "City_04_02", "maps/City_Zones/City_04_02/City_04_02.txt", "Brickstown"},
    {10, "City_05_01", "maps/City_Zones/City_05_01/City_05_01.txt", "Peregrine Island"},

    // Hazards
    {11, "Hazard_01_01", "maps/City_Zones/Hazard_01_01/Hazard_01_01.txt", "Perez Park"},
    {12, "Hazard_02_01", "maps/City_Zones/Hazard_02_01/Hazard_02_01.txt", "Boomtown"},
    {13, "Hazard_03_01", "maps/City_Zones/Hazard_03_01/Hazard_03_01.txt", "Dark Astoria"},
    {14, "Hazard_04_01", "maps/City_Zones/Hazard_04_01/Hazard_04_01.txt", "Crey's Folly"},
    {15, "Hazard_04_02", "maps/City_Zones/Hazard_04_02/Hazard_04_02.txt", "Enviro Nightmare"},
    {16, "Hazard_05_01", "maps/City_Zones/Hazard_05_01/Hazard_05_01.txt", "Elysium"},

    // Trials
    {17, "Trial_01_01", "maps/City_Zones/Trial_01_01/Trial_01_01.txt", "Abandoned Sewer Network"},
    {18, "Trial_01_02", "maps/City_Zones/Trial_01_02/Trial_01_02.txt", "Sewer Network"},
    {19, "Trial_02_01", "maps/City_Zones/Trial_02_01/Trial_02_01.txt", "Faultline"},
    {20, "Trial_03_01", "maps/City_Zones/Trial_03_01/Trial_03_01.txt", "Terra Volta"},
    {21, "Trial_04_01", "maps/City_Zones/Trial_04_01/Trial_04_01.txt", "Eden"},
    {22, "Trial_04_02", "maps/City_Zones/Trial_04_02/Trial_04_02.txt", "The Hive"},
    {23, "Trial_05_01", "maps/City_Zones/Trial_05_01/Trial_05_01.txt", "Rikti Crash Site"}
};

const std::vector<MapData> getAllMapData()
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
        if (map_name.contains(map_data.m_map_name, Qt::CaseInsensitive))
            return map_data.m_map_idx;
    }

    // log a warning because this part of the code is called when things went wrong
    qWarning() << "No matching map name in g_defined_map_datas to sent map name."
               << "Returning Outbreak's map index as default...";

    // defaulting to Outbreak's map name
    return 0;
}

QString getDisplayMapName(const QString &map_name)
{
    for (const auto &map_data : g_defined_map_datas)
    {
        if (map_name.contains(map_data.m_map_name, Qt::CaseInsensitive))
            return map_data.m_display_map_name;
    }

    // log a warning because this part of the code is called when things went wrong
    qWarning() << "No matching map name in g_defined_map_datas to sent map name."
               << "Returning Outbreak's display map name as default...";

    // defaulting to Outbreak's map name
    return g_defined_map_datas[0].m_display_map_name;
}

QString getDisplayMapName(size_t index)
{
    // Since index is unsigned, it cannot be negative.
    // Thus, no need to check for index < 0.
    if(index >= g_defined_map_datas.size())
    {
        qWarning() << "Sought map index was out of range."
                   << "Returning Outbreak's display map name as default...";
        index = 0;
    }
    return g_defined_map_datas[index].m_display_map_name;
}

QString getMapName(size_t index)
{
    // Since index is unsigned, it cannot be negative.
    // Thus, no need to check for index < 0.
    if(index >= g_defined_map_datas.size())
    {
        qWarning() << "Sought map index was out of range."
                   << "Returning Outbreak's map name as default...";
        index = 0;
    }
    return g_defined_map_datas[index].m_map_name;
}

QString getMapPath(size_t index)
{
    if(index >= g_defined_map_datas.size()){
        qWarning() << "Sought map index was out of range."
                   << "Returning Outbreak's map path as default...";
        index = 0;
    }
    return g_defined_map_datas[index].m_map_path;
}
