--Dark Astoria ELO

include_lua('./Spawndefs/Banished_DarkAstoria.spawndef.lua')
include_lua('./Spawndefs/CircleOfThorns_DarkAstoria.spawndef.lua')
include_lua('./Spawndefs/Tsoo_DarkAstoria.spawndef.lua')

ES_Library_Objects = {
      ["ES_BP_ZombieLoiter_D1_Phalanx_Haz_03_01"] = {
         ["EncounterSpawn"] = "Phalanx",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Loiter_BansihedPantheon_D1_V0,Loiter_BansihedPantheon_D1_V1,Loiter_BansihedPantheon_D1_V2}
      },
      ["ES_BP_ZombieLoiter_D5_Phalanx_Haz_03_01"] = {
         ["EncounterSpawn"] = "Phalanx",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Loiter_BansihedPantheon_D5_V0,Loiter_BansihedPantheon_D5_V1,Loiter_BansihedPantheon_D5_V2}
      },
      ["ES_BP_ZombieLoiter_D10_Phalanx_Haz_03_01"] = {
         ["EncounterSpawn"] = "Phalanx",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Loiter_BansihedPantheon_D10_V0,Loiter_BansihedPantheon_D10_V1,Loiter_BansihedPantheon_D10_V2}
      },
      ["ES_BanishedPantheon_D3_Around_Haz_03_01"] = {
         ["EncounterSpawn"] = "Around",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn7","CanSpawn8","CanSpawn9"},
         ["CanSpawnDefs"] = {Ceremony_BanishedPantheon_D3_V0, Ceremony_BanishedPantheon_D3_V1, Ceremony_BanishedPantheon_D3_V2, 
                              GhostSacrifice_BanishedPantheon_D3_V0, GhostSacrifice_BanishedPantheon_D3_V1, GhostSacrifice_BanishedPantheon_D3_V2,
                              Summoning_BanishedPantheon_D3_V0, Summoning_BanishedPantheon_D3_V1, Summoning_BanishedPantheon_D3_V2}
      },
      ["ES_BanishedPantheon_D8_Around_Haz_03_01"] = {
         ["EncounterSpawn"] = "Around",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6","CanSpawn7","CanSpawn8","CanSpawn9"},
         ["CanSpawnDefs"] = {Ceremony_BanishedPantheon_D8_V0, Ceremony_BanishedPantheon_D8_V1, Ceremony_BanishedPantheon_D8_V2,
                              GhostSacrifice_BanishedPantheon_D8_V0, GhostSacrifice_BanishedPantheon_D8_V1, GhostSacrifice_BanishedPantheon_D8_V2,
                              Summoning_BanishedPantheon_D8_V0, Summoning_BanishedPantheon_D8_V1, Summoning_BanishedPantheon_D8_V2}
      },
      ["ES_CircleOfThorns_D3_Around_Haz_03_01"] = {         
         ["EncounterSpawn"] = "Around",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Ceremony_CircleOfThorns_D3_V0,Ceremony_CircleOfThorns_D3_V1,Ceremony_CircleOfThorns_D3_V2}
      },
      ["ES_CircleOfThorns_D8_Around_Haz_03_01"] = {
         ["EncounterSpawn"] = "Around",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Ceremony_CircleOfThorns_D8_V0,Ceremony_CircleOfThorns_D8_V1,Ceremony_CircleOfThorns_D8_V2}
      },
      ["ES_CoTPantheon_D1_Rumble_Haz_03_01"] = {
         ["EncounterSpawn"] = "Rumble",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Rumble_CoTPantheon_D1_V0,Rumble_CoTPantheon_D1_V1,Rumble_CoTPantheon_D1_V2}
      },
      ["ES_CoTPantheon_D5_Rumble_Haz_03_01"] = {
         ["EncounterSpawn"] = "Rumble",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Rumble_CoTPantheon_D5_V0,Rumble_CoTPantheon_D5_V1,Rumble_CoTPantheon_D5_V2}
      },
      ["ES_CoTPantheon_D10_Rumble_Haz_03_01"] = {
         ["EncounterSpawn"] = "Rumble",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Rumble_CoTPantheon_D10_V0,Rumble_CoTPantheon_D10_V1,Rumble_CoTPantheon_D10_V2}
      },
      ["ES_Tsoo_D1_Around_Haz_03_01"] = {
         ["EncounterSpawn"] = "Around",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Investigate_Tsoo_D1_V0,Investigate_Tsoo_D1_V1,Investigate_Tsoo_D1_V2}
      },
      ["ES_Tsoo_D5_Around_Haz_03_01"] = {
         ["EncounterSpawn"] = "Around",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Investigate_Tsoo_D5_V0,Investigate_Tsoo_D5_V1,Investigate_Tsoo_D5_V2}
      },
      ["ES_Tsoo_D10_Around_Haz_03_01"] = {
         ["EncounterSpawn"] = "Around",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Investigate_Tsoo_D10_V0,Investigate_Tsoo_D10_V1,Investigate_Tsoo_D10_V2}
      },
      ["ES_TsooPantheon_D1_Rumble_Haz_03_01"] = {
         ["EncounterSpawn"] = "Rumble",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Rumble_TsooPantheon_D1_V0,Rumble_TsooPantheon_D1_V1,Rumble_TsooPantheon_D1_V2}
      },
      ["ES_TsooPantheon_D5_Rumble_Haz_03_01"] = {
         ["EncounterSpawn"] = "Rumble",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Rumble_TsooPantheon_D5_V0,Rumble_TsooPantheon_D5_V1,Rumble_TsooPantheon_D5_V2}
      },
      ["ES_TsooPantheon_D10_Rumble_Haz_03_01"] = {
         ["EncounterSpawn"] = "Rumble",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {Rumble_TsooPantheon_D10_V0,Rumble_TsooPantheon_D10_V1,Rumble_TsooPantheon_D10_V2}
      },
      ["ES_RisingDead_D10_frombelow_Haz_03_01"] = {
         ["EncounterSpawn"] = "ZombieBurst1",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {FromBelow_BanishedPantheon_D10_V0,FromBelow_BanishedPantheon_D10_V1,FromBelow_BanishedPantheon_D10_V2}
      },
      ["ES_RisingDead_D10_frombelow_02_Haz_03_01"] = {
         ["EncounterSpawn"] = "ZombieBurst2",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {FromBelow_BanishedPantheon_D10_V0,FromBelow_BanishedPantheon_D10_V1,FromBelow_BanishedPantheon_D10_V2}
      },
      ["ES_RisingDead_D10_frombelow_03_Haz_03_01"] = {
         ["EncounterSpawn"] = "ZombieBurst3",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {FromBelow_BanishedPantheon_D10_V0,FromBelow_BanishedPantheon_D10_V1,FromBelow_BanishedPantheon_D10_V2}
      },
      ["ES_RisingDead_D10_frombelow_04_Haz_03_01"] = {
         ["EncounterSpawn"] = "ZombieBurst4",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {FromBelow_BanishedPantheon_D10_V0,FromBelow_BanishedPantheon_D10_V1,FromBelow_BanishedPantheon_D10_V2}
      },
      ["ES_BP_xx_frombelow_01_Haz_03_01"] = {
         ["EncounterSpawn"] = "ZombieBurst1",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {FromBelow_BanishedPantheon_D10_V0,FromBelow_BanishedPantheon_D10_V1,FromBelow_BanishedPantheon_D10_V2}
      },
      ["ES_BP_xx_frombelow_02_Haz_03_01"] = {
         ["EncounterSpawn"] = "ZombieBurst2",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {FromBelow_BanishedPantheon_D10_V0,FromBelow_BanishedPantheon_D10_V1,FromBelow_BanishedPantheon_D10_V2}
      },
      ["ES_BP_xx_frombelow_03_Haz_03_01"] = {
         ["EncounterSpawn"] = "ZombieBurst3",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {FromBelow_BanishedPantheon_D10_V0,FromBelow_BanishedPantheon_D10_V1,FromBelow_BanishedPantheon_D10_V2}
      },
      ["ES_BP_xx_frombelow_04_Haz_03_01"] = {
         ["EncounterSpawn"] = "ZombieBurst4",
         ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3"},
         ["CanSpawnDefs"] = {FromBelow_BanishedPantheon_D10_V0,FromBelow_BanishedPantheon_D10_V1,FromBelow_BanishedPantheon_D10_V2}
      },
      ["ES_Effigy_Haz_03_01"] = {
         ["SpawnProbability"] = 100,
         ["EncounterSpawn"] = "Encounter",
         ["CanSpawn"] = {"CanSpawn1"},
         ["CanSpawnDefs"] = {Effigy_D10_V0}
      },
}

--REQUIRED: This inserts universal into this map's OL
local UniOnce
if UniOnce == false or UniOnce == nil then
      InsertUniversals()
      UniOnce = true
end