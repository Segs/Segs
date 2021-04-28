#include "spawn_definitions.h"

#include "Components/Logging.h"


void SpawnDefinitions::buildList()
{
    CritterSpawnDef* list = new CritterSpawnDef();
    //Empty
    list->m_spawn_group =  "None";
    m_critter_spawn_list.push_back(*list);

    //Outbreak
    list->m_spawn_group =  "ES_Contaminated_City_00_01";
    list->m_possible_critters.push_back({ "Thug_Contaminated_01", "Contaminated Thug", "Contaminated", false });
    list->m_possible_critters.push_back({ "Thug_Contaminated_02", "Contaminated Slicer", "Contaminated", false });
    list->m_possible_critters.push_back({ "Thug_Contaminated_03", "Contaminated Scavenger", "Contaminated", false });
    list->m_possible_critters.push_back({ "Thug_Contaminated_04", "Contaminated Thug", "Contaminated", false });
    list->m_possible_critters.push_back({ "Thug_Contaminated_05", "Contaminated Slicer", "Contaminated", false });
    list->m_possible_critters.push_back({ "Thug_Contaminated_06", "Contaminated Scavenger", "Contaminated", false });
    list->m_possible_critters.push_back({ "Thug_Contaminated_07", "Contaminated Thug", "Contaminated", false });
    list->m_possible_critters.push_back({ "Thug_Contaminated_08", "Contaminated Slicer", "Contaminated", false });
    list->m_possible_critters.push_back({ "Thug_Contaminated_09", "Contaminated Scavenger", "Contaminated", false });
    list->m_possible_critters.push_back({ "Thug_Contaminated_10", "Contaminated Thug", "Contaminated", false });

    m_critter_spawn_list.push_back(*list);

    list = new CritterSpawnDef();
    list->m_spawn_group = "ES_Robot1_City_00_01";
    list->m_possible_critters.push_back({ "RiktiDrone_Down", "Destroyed Drone", "Rikti", true });
    m_critter_spawn_list.push_back(*list);

    list = new CritterSpawnDef();
    list->m_spawn_group = "ES_Robot2_City_00_01";
    list->m_possible_critters.push_back({ "RiktiDrone_1", "Practice Drone", "Rikti", true });
    list->m_possible_critters.push_back({ "RiktiDrone_2", "Practice Drone", "Rikti", true });
    list->m_possible_critters.push_back({ "RiktiDrone_3", "Practice Drone", "Rikti", true });
    m_critter_spawn_list.push_back(*list);

    list = new CritterSpawnDef();
    list->m_spawn_group = "ES_Robot3_City_00_01";
    list->m_possible_critters.push_back({ "RiktiDrone_1", "Practice Drone", "Rikti", true });
    list->m_possible_critters.push_back({ "RiktiDrone_2", "Practice Drone", "Rikti", true });
    list->m_possible_critters.push_back({ "RiktiDrone_3", "Practice Drone", "Rikti", true });
    m_critter_spawn_list.push_back(*list);

    //Atlas

}
/*
static const SpawnNPCList g_defined_enemy_spawn_groups =
{
    //Outbreak

    //Atlas
    {
        "EG_L1_3_Around_City_01_01",
        {
                "Thug_Hellion_01",
                "Thug_Hellion_02",
                "Thug_Hellion_03",
                "Thug_Hellion_04",
                "Thug_Hellion_05",
                "Thug_Hellion_06"}
    },
    {
        "EG_L1_3_PurseSnatch_City_01_01",
        {
                "Thug_Hellion_01",
                "Thug_Hellion_02",
                "Thug_Hellion_03",
                "Thug_Hellion_04",
                "Thug_Hellion_05",
                "Thug_Hellion_06"
        }
    },
    {
        "EG_L2_5_AroundDoor_City_01_01",
        {
                "Thug_Hellion_01",
                "Thug_Hellion_02",
                "Thug_Hellion_03",
                "Thug_Hellion_04",
                "Thug_Hellion_05",
                "Thug_Hellion_06"
        }
    }
};*/

std::vector<CritterSpawnDef> SpawnDefinitions::getCritterSpawnDefinitions()
{
    if(m_critter_spawn_list.size() < 1)
    {
        buildList();
    }

    return m_critter_spawn_list;
}

CritterSpawnDef SpawnDefinitions::getSpawnGroup(const QString &spawn_group_name)
{
    if(m_critter_spawn_list.size() < 1)
    {
        buildList();
    }

    for(const auto &spawn_group : m_critter_spawn_list)
    {
        if(spawn_group.m_spawn_group.contains(spawn_group_name, Qt::CaseInsensitive))
            return spawn_group;
    }
    qCDebug(logNpcSpawn) << "No matching \"" << spawn_group_name << "\" in g_defined_enemy_spawn_groups to sent group name."
               << "Returning Empty group...";

    return m_critter_spawn_list[0];
}
