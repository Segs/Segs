--Perez Park

--These contain tables for relevant, spawnable NPCs
include_lua('./Spawndefs/Hellions_Perez.spawndef.lua')
include_lua('./Spawndefs/Skulls_Perez.spawndef.lua')
include_lua('./Spawndefs/Clockwork_Perez.spawndef.lua')
include_lua('./Spawndefs/Vahzilok_Perez.spawndef.lua')
include_lua('./Spawndefs/Rikti_Perez.spawndef.lua')
include_lua('./Spawndefs/Hydramen_Perez.spawndef.lua')
include_lua('./Spawndefs/CircleOfThorns_Perez.spawndef.lua')

ES_Library_Objects = {
  ["ES_Hellions_L1_3_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5"},
    ["CanSpawnDefs"] = {Loiter_Hellions_L1_3_V0, Loiter_Hellions_L1_3_V1, Loiter_Hellions_L1_3_V2, Loiter_Hellions_L1_3_V3, Loiter_Hellions_L1_3_V4},
  },
  ["ES_Hellions_L4_7_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5"},
    ["CanSpawnDefs"] = {Loiter_Hellions_L4_7_V0, Loiter_Hellions_L4_7_V1, Loiter_Hellions_L4_7_V2, Loiter_Hellions_L4_7_V3, Loiter_Hellions_L4_7_V4},
  },
  ["ES_Hellions_L8_10_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5"},
    ["CanSpawnDefs"] = {Loiter_Hellions_L8_10_V0, Loiter_Hellions_L8_10_V1, Loiter_Hellions_L8_10_V2, Loiter_Hellions_L8_10_V3, Loiter_Hellions_L8_10_V4},
  },
  ["ES_Skulls_L1_3_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5"},
    ["CanSpawnDefs"] = {Loiter_Skulls_L1_3_V0, Loiter_Skulls_L1_3_V1, Loiter_Skulls_L1_3_V2, Loiter_Skulls_L1_3_V3, Loiter_Skulls_L1_3_V4},
  }, 
  ["ES_Skulls_L4_7_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5"},
    ["CanSpawnDefs"] = {Loiter_Skulls_L4_7_V0, Loiter_Skulls_L4_7_V1, Loiter_Skulls_L4_7_V2, Loiter_Skulls_L4_7_V3, Loiter_Skulls_L4_7_V4},
  },
  ["ES_Skulls_L8_10_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5"},
    ["CanSpawnDefs"] = {Loiter_Skulls_L8_10_V0, Loiter_Skulls_L8_10_V1, Loiter_Skulls_L8_10_V2, Loiter_Skulls_L8_10_V3, Loiter_Skulls_L8_10_V4},
  },
  ["ES_Clockwork_L1_3_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",
                    "CanSpawn6","CanSpawn7","CanSpawn8","CanSpawn9","CanSpawn10","CanSpawn11",
                    "CanSpawn12","CanSpawn13","CanSpawn14","CanSpawn15","CanSpawn16"},
    ["CanSpawnDefs"] = {Scavenger_Clockwork_L1_3_V0, Scavenger_Clockwork_L1_3_V1, 
                        Scavenger_Clockwork_L1_3_V2, Scavenger_Clockwork_L1_3_V3, 
                        Scavenger_Clockwork_L1_3_V4, Scavenger_Clockwork_L1_3_V5, 
                        Scavenger_Clockwork_L1_3_V6, Scavenger_Clockwork_L1_3_V7, 
                        Scavenger_Clockwork_L1_3_V8, Scavenger_Clockwork_L1_3_V9, 
                        Scavenger_Clockwork_L1_3_V10, Scavenger_Clockwork_L1_3_V11, 
                        Scavenger_Clockwork_L1_3_V12, Scavenger_Clockwork_L1_3_V13, 
                        Scavenger_Clockwork_L1_3_V14, Scavenger_Clockwork_L1_3_V15,},
  },
  ["ES_Clockwork_L4_7_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",
                    "CanSpawn6","CanSpawn7","CanSpawn8","CanSpawn9","CanSpawn10","CanSpawn11",
                    "CanSpawn12","CanSpawn13","CanSpawn14","CanSpawn15","CanSpawn16"},
    ["CanSpawnDefs"] = {Scavenger_Clockwork_L4_7_V0, Scavenger_Clockwork_L4_7_V1, 
                        Scavenger_Clockwork_L4_7_V2, Scavenger_Clockwork_L4_7_V3, 
                        Scavenger_Clockwork_L4_7_V4, Scavenger_Clockwork_L4_7_V5, 
                        Scavenger_Clockwork_L4_7_V6, Scavenger_Clockwork_L4_7_V7, 
                        Scavenger_Clockwork_L4_7_V8, Scavenger_Clockwork_L4_7_V9, 
                        Scavenger_Clockwork_L4_7_V10, Scavenger_Clockwork_L4_7_V11, 
                        Scavenger_Clockwork_L4_7_V12, Scavenger_Clockwork_L4_7_V13, 
                        Scavenger_Clockwork_L4_7_V14, Scavenger_Clockwork_L4_7_V15,},
  },
  ["ES_Clockwork_L8_10_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",
                    "CanSpawn6","CanSpawn7","CanSpawn8","CanSpawn9","CanSpawn10","CanSpawn11",
                    "CanSpawn12","CanSpawn13","CanSpawn14","CanSpawn15","CanSpawn16"},
    ["CanSpawnDefs"] = {Scavenger_Clockwork_L8_10_V0, Scavenger_Clockwork_L8_10_V1, 
                        Scavenger_Clockwork_L8_10_V2, Scavenger_Clockwork_L8_10_V3, 
                        Scavenger_Clockwork_L8_10_V4, Scavenger_Clockwork_L8_10_V5, 
                        Scavenger_Clockwork_L8_10_V6, Scavenger_Clockwork_L8_10_V7, 
                        Scavenger_Clockwork_L8_10_V8, Scavenger_Clockwork_L8_10_V9, 
                        Scavenger_Clockwork_L8_10_V10, Scavenger_Clockwork_L8_10_V11, 
                        Scavenger_Clockwork_L8_10_V12, Scavenger_Clockwork_L8_10_V13, 
                        Scavenger_Clockwork_L8_10_V14, Scavenger_Clockwork_L8_10_V15,},
  },
  ["ES_CoT_L2_5_Ambush_Haz_01_01_1&"] = {
    ["EncounterSpawn"] = "Ambush",
    ["CanSpawn"] = {},
    ["CanSpawnDefs"] = {},
  },
  ["ES_CoT_L2_5_Ambush_Haz_01_01_2&"] = {
    ["EncounterSpawn"] = "Ambush",
    ["CanSpawn"] = {},
    ["CanSpawnDefs"] = {},
  },
  ["ES_CoT_L2_5_Ambush_Haz_01_01_3&"] = {
    ["EncounterSpawn"] = "Ambush",
    ["CanSpawn"] = {},
    ["CanSpawnDefs"] = {},
  },
  ["ES_CoT_L2_5_Ambush_Haz_01_01_4&"] = {
    ["EncounterSpawn"] = "Ambush",
    ["CanSpawn"] = {},
    ["CanSpawnDefs"] = {},
  },
  ["ES_CoT_L2_5_Ambush_Haz_01_01&"] = {
    ["EncounterSpawn"] = "Ambush",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",
                    "CanSpawn6","CanSpawn7","CanSpawn8","CanSpawn9","CanSpawn10","CanSpawn11",
                    "CanSpawn12","CanSpawn13","CanSpawn14","CanSpawn15","CanSpawn16"},
    ["CanSpawnDefs"] = {Ambush_CircleOfThornsFollowers_L2_5_V0, Scavenger_Clockwork_L2_5_V1, 
              Ambush_CircleOfThornsFollowers_L2_5_V2, Ambush_CircleOfThornsFollowers_L2_5_V3, 
              Ambush_CircleOfThornsFollowers_L2_5_V4, Ambush_CircleOfThornsFollowers_L2_5_V5, 
              Ambush_CircleOfThornsFollowers_L2_5_V6, Ambush_CircleOfThornsFollowers_L2_5_V7, 
              Ambush_CircleOfThornsFollowers_L2_5_V8, Ambush_CircleOfThornsFollowers_L2_5_V9, 
              Ambush_CircleOfThornsFollowers_L2_5_V10, Ambush_CircleOfThornsFollowers_L2_5_V11, 
              Ambush_CircleOfThornsFollowers_L2_5_V12, Ambush_CircleOfThornsFollowers_L2_5_V13, 
              Ambush_CircleOfThornsFollowers_L2_5_V14, Ambush_CircleOfThornsFollowers_L2_5_V15,},
  },
  ["ES_CoT_L5_9_Ambush_Haz_01_01&"] = {
    ["EncounterSpawn"] = "Ambush",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",
                    "CanSpawn6","CanSpawn7","CanSpawn8","CanSpawn9","CanSpawn10","CanSpawn11",
                    "CanSpawn12","CanSpawn13","CanSpawn14","CanSpawn15","CanSpawn16"},
    ["CanSpawnDefs"] = {Ambush_CircleOfThornsFollowers_L5_9_V0, Scavenger_Clockwork_L5_9_V1, 
              Ambush_CircleOfThornsFollowers_L5_9_V2, Ambush_CircleOfThornsFollowers_L5_9_V3, 
              Ambush_CircleOfThornsFollowers_L5_9_V4, Ambush_CircleOfThornsFollowers_L5_9_V5, 
              Ambush_CircleOfThornsFollowers_L5_9_V6, Ambush_CircleOfThornsFollowers_L5_9_V7, 
              Ambush_CircleOfThornsFollowers_L5_9_V8, Ambush_CircleOfThornsFollowers_L5_9_V9, 
              Ambush_CircleOfThornsFollowers_L5_9_V10, Ambush_CircleOfThornsFollowers_L5_9_V11, 
              Ambush_CircleOfThornsFollowers_L5_9_V12, Ambush_CircleOfThornsFollowers_L5_9_V13, 
              Ambush_CircleOfThornsFollowers_L5_9_V14, Ambush_CircleOfThornsFollowers_L5_9_V15,},
  },
  ["ES_Vah_L2_5_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3",},
    ["CanSpawnDefs"] = {Loiter_Vahzilok_L2_5_V0, Loiter_Vahzilok_L2_5_V1, Loiter_Vahzilok_L2_5_V2},
  },
  ["ES_Vah_L5_9_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3",},
    ["CanSpawnDefs"] = {Loiter_Vahzilok_L5_9_V0, Loiter_Vahzilok_L5_9_V1, Loiter_Vahzilok_L5_9_V2},
  },
  ["ES_RiktiBeggar_L8_10_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3",},
    ["CanSpawnDefs"] = {Preaching_RiktiBeggar_L8_10_V0, Preaching_RiktiBeggar_L8_10_V1, Preaching_RiktiBeggar_L8_10_V2},
  },
  ["ES_HydraMen_L7_9_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",
                    "CanSpawn6","CanSpawn7","CanSpawn8","CanSpawn9","CanSpawn10","CanSpawn11",
                    "CanSpawn12","CanSpawn13","CanSpawn14","CanSpawn15","CanSpawn16"},
    ["CanSpawnDefs"] = {Loiter_HydraMan_L7_9_V0, Loiter_HydraMan_L7_9_V1, 
                        Loiter_HydraMan_L7_9_V2, Loiter_HydraMan_L7_9_V3, 
                        Loiter_HydraMan_L7_9_V4, Loiter_HydraMan_L7_9_V5, 
                        Loiter_HydraMan_L7_9_V6, Loiter_HydraMan_L7_9_V7, 
                        Loiter_HydraMan_L7_9_V8, Loiter_HydraMan_L7_9_V9, 
                        Loiter_HydraMan_L7_9_V10, Loiter_HydraMan_L7_9_V11, 
                        Loiter_HydraMan_L7_9_V12, Loiter_HydraMan_L7_9_V13, 
                        Loiter_HydraMan_L7_9_V14, Loiter_HydraMan_L7_9_V15,},
  },
  ["ES_HellionSkull_L4_7_Rumble_Haz_01_01"] = {
    ["EncounterSpawn"] = "Rumble",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3",},
    ["CanSpawnDefs"] = {Rumble_HellionsSkulls_L4_7_V0, Rumble_HellionsSkulls_L4_7_V1, Rumble_HellionsSkulls_L4_7_V2},
  },        
  ["ES_HellionSkull_L8_10_Rumble_Haz_01_01"] = {
    ["EncounterSpawn"] = "Rumble",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3",},
    ["CanSpawnDefs"] = {Rumble_HellionsSkulls_L8_10_V0, Rumble_HellionsSkulls_L8_10_V1, Rumble_HellionsSkulls_L8_10_V2},
  },
  ["ES_CoT_L2_5_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",
                    "CanSpawn6","CanSpawn7","CanSpawn8","CanSpawn9","CanSpawn10","CanSpawn11",
                    "CanSpawn12","CanSpawn13","CanSpawn14","CanSpawn15","CanSpawn16"},
    ["CanSpawnDefs"] = {Sacrifice_CircleOfThornsFollowe_L2_5_V0, Sacrifice_CircleOfThornsFollowe_L2_5_V1, 
                        Sacrifice_CircleOfThornsFollowe_L2_5_V2, Sacrifice_CircleOfThornsFollowe_L2_5_V3, 
                        Sacrifice_CircleOfThornsFollowe_L2_5_V4, Sacrifice_CircleOfThornsFollowe_L2_5_V5, 
                        Sacrifice_CircleOfThornsFollowe_L2_5_V6, Sacrifice_CircleOfThornsFollowe_L2_5_V7, 
                        Sacrifice_CircleOfThornsFollowe_L2_5_V8, Sacrifice_CircleOfThornsFollowe_L2_5_V9, 
                        Sacrifice_CircleOfThornsFollowe_L2_5_V10, Sacrifice_CircleOfThornsFollowe_L2_5_V11, 
                        Sacrifice_CircleOfThornsFollowe_L2_5_V12, Sacrifice_CircleOfThornsFollowe_L2_5_V13, 
                        Sacrifice_CircleOfThornsFollowe_L2_5_V14, Sacrifice_CircleOfThornsFollowe_L2_5_V15,},
  },
  ["ES_CoT_L5_9_Around_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",
                    "CanSpawn6","CanSpawn7","CanSpawn8","CanSpawn9","CanSpawn10","CanSpawn11",
                    "CanSpawn12","CanSpawn13","CanSpawn14","CanSpawn15","CanSpawn16"},
    ["CanSpawnDefs"] = {Sacrifice_CircleOfThornsFollowe_L5_9_V0, Sacrifice_CircleOfThornsFollowe_L5_9_V1, 
                        Sacrifice_CircleOfThornsFollowe_L5_9_V2, Sacrifice_CircleOfThornsFollowe_L5_9_V3, 
                        Sacrifice_CircleOfThornsFollowe_L5_9_V4, Sacrifice_CircleOfThornsFollowe_L5_9_V5, 
                        Sacrifice_CircleOfThornsFollowe_L5_9_V6, Sacrifice_CircleOfThornsFollowe_L5_9_V7, 
                        Sacrifice_CircleOfThornsFollowe_L5_9_V8, Sacrifice_CircleOfThornsFollowe_L5_9_V9, 
                        Sacrifice_CircleOfThornsFollowe_L5_9_V10, Sacrifice_CircleOfThornsFollowe_L5_9_V11, 
                        Sacrifice_CircleOfThornsFollowe_L5_9_V12, Sacrifice_CircleOfThornsFollowe_L5_9_V13, 
                        Sacrifice_CircleOfThornsFollowe_L5_9_V14, Sacrifice_CircleOfThornsFollowe_L5_9_V15,},
  },
  ["ES_Baphomet_Haz_01_01"] = {             --Defined in CircleOfThorns_Perez.spawndef.lua
    ["EncounterSpawn"] = "Encounter",
    ["CanSpawn"] = {"CanSpawn1",},
    ["CanSpawnDefs"] = {Baphomet_D10_V0},
  },
  ["ES_Clockwork_L5_9_Flying_Haz_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1",},
    ["CanSpawnDefs"] = {Flying_Clockwork_L5_9_V0},
  },        
}

--REQUIRED: This inserts universal into this map's OL
local UniOnce
if UniOnce == false or UniOnce == nil then
    InsertUniversals()
    UniOnce = true
end