--Provide spawndefs tables for relevant CanSpawnDefs here
include_lua('./Spawndefs/Hellions_Atlas.spawndef.lua')
include_lua('./Spawndefs/Clockwork_Atlas.spawndef.lua')
include_lua('./Spawndefs/Vahzilok_Atlas.spawndef.lua')

ES_Library_Objects = {
  ["_ES_L1_3_Around_City_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn2","CanSpawn3",},
    ["CanSpawnDefs"] = {Mugging_Thugs_L1_3_V0, Mugging_Thugs_L1_3_V1, ShadyDeal_Thugs_L1_3_V0, ShadyDeal_Thugs_L1_3_V1},
  },
  ["_ES_L1_3_Rooftop_City_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn2"},
    ["CanSpawnDefs"] = {ShadyDeal_Thugs_L1_3_V0, ShadyDeal_Thugs_L1_3_V1,},
  },
  ["_ES_L4_7_Around_City_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn2","CanSpawn3",},
    ["CanSpawnDefs"] = {Mugging_Thugs_L4_7_V0, Mugging_Thugs_L4_7_V1, ShadyDeal_Thugs_L4_7_V0, ShadyDeal_Thugs_L4_7_V1,},
  },
  ["_ES_L4_7_Rooftop_City_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn2"},
    ["CanSpawnDefs"] = {ShadyDeal_Thugs_L4_7_V0, ShadyDeal_Thugs_L4_7_V1,},
  },
  ["_ES_L8_10_Around_City_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn2"},
    ["CanSpawnDefs"] = {Mugging_Thugs_L8_10_V0, Mugging_Thugs_L8_10_V1,},
  },
  ["_ES_L8_10_Rooftop_City_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn2"},
    ["CanSpawnDefs"] = {ShadyDeal_Thugs_L8_10_V0, ShadyDeal_Thugs_L8_10_V1,},
  },
  ["_ES_Clockwork_L4_7_Around_City_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn1"},
    ["CanSpawnDefs"] = {Scavenge_Clockwork_L4_7_V0, Scavenge_Clockwork_L4_7_V1,},
  },
  ["_ES_Clockwork_L8_10_Around_City_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn1"},
    ["CanSpawnDefs"] = {Scavenge_Clockwork_L8_10_V0, Scavenge_Clockwork_L8_10_V1,},
  },
  ["_ES_L1_3_AroundVandalism_City_01_01"] = {
    ["EncounterSpawn"] = "AroundVandalism",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn1"},
    ["CanSpawnDefs"] = {Vandalism_Thugs_L1_3_V0, Vandalism_Thugs_L1_3_V1,},
  },
  ["_ES_L4_7_AroundVandalism_City_01_01"] = {
    ["EncounterSpawn"] = "AroundVandalism",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn1"},
    ["CanSpawnDefs"] = {Vandalism_Thugs_L4_7_V0, Vandalism_Thugs_L4_7_V1,},
  },
  ["_ES_L8_10_AroundVandalism_City_01_01"] = {
    ["EncounterSpawn"] = "AroundVandalism",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn1"},
    ["CanSpawnDefs"] = {Vandalism_Thugs_L8_10_V0, Vandalism_Thugs_L8_10_V1,},
  },
  ["_ES_L1_3_AroundDoor_City_01_01"] = {
    ["EncounterSpawn"] = "AroundDoor",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn1"},
    ["CanSpawnDefs"] = {BreakingIn_Thugs_L2_5_V0, BreakingIn_Thugs_L2_5_V1,},
  },
  ["_ES_L2_5_AroundDoor_City_01_01"] = {
    ["EncounterSpawn"] = "AroundDoor",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn1"},
    ["CanSpawnDefs"] = {BreakingIn_Thugs_L2_5_V0, BreakingIn_Thugs_L2_5_V1,},
  },
  ["_ES_L6_9_AroundDoor_City_01_01"] = {
    ["EncounterSpawn"] = "AroundDoor",
    ["CanSpawn"] = {"CanSpawn1","CanSpawn1"},
    ["CanSpawnDefs"] = {BreakingIn_Thugs_L6_9_V0, BreakingIn_Thugs_L6_9_V1,},
  },
  ["_ES_L1_3_Ambush_City_01_01"] = {
    ["EncounterSpawn"] = "Ambush",
    ["CanSpawn"] = {"CanSpawn1",},
    ["CanSpawnDefs"] = {Ambush_Thugs_L1_3},
  },
  ["_ES_Rikti_Ambush_City_01_01"] = {
    ["EncounterSpawn"] = "Ambush",
    ["CanSpawn"] = {"CanSpawn1",},
    ["CanSpawnDefs"] = {Invasion_Rikti_L1_3},
  },
  ["_ES_Rikti_Ambush_City_01_01"] = {
    ["EncounterSpawn"] = "Ambush",
    ["CanSpawn"] = {"CanSpawn1",},
    ["CanSpawnDefs"] = {Invasion_Rikti_L1_3},
  },
  ["_ES_L1_3_PurseSnatch_City_01_01"] = {
    ["EncounterSpawn"] = "Snatch",
    ["CanSpawn"] = {"CanSpawn1"},
    ["CanSpawnDefs"] = {PurseSnatch_Thugs_L1_3_V0,},
  },
  ["_ES_L4_7_PurseSnatch_City_01_01"] = {
    ["EncounterSpawn"] = "Snatch",
    ["CanSpawn"] = {"CanSpawn1",},
    ["CanSpawnDefs"] = {PurseSnatch_Thugs_L4_7_V0,},
  },
  ["_ES_L8_10_PurseSnatch_City_01_01"] = {
    ["EncounterSpawn"] = "Snatch",
    ["CanSpawn"] = {"CanSpawn1",},
    ["CanSpawnDefs"] = {PurseSnatch_Thugs_L8_10_V0,},
  },
  ["_ES_Vahzilok_D1_Around_City_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2", "CanSpawn3"},
    ["CanSpawnDefs"] = {Loiter_Vahzilok_D1_V0, Loiter_Vahzilok_D1_V1, Loiter_Vahzilok_D1_V2,},
  },
  ["_ES_Vahzilok_D5_Around_City_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2", "CanSpawn3"},
    ["CanSpawnDefs"] = {Loiter_Vahzilok_D5_V0, Loiter_Vahzilok_D5_V1, Loiter_Vahzilok_D5_V2,},
  },
  ["_ES_Vahzilok_D10_Around_City_01_01"] = {
    ["EncounterSpawn"] = "Around",
    ["CanSpawn"] = {"CanSpawn1", "CanSpawn2", "CanSpawn3"},
    ["CanSpawnDefs"] = {Loiter_Vahzilok_D10_V0, Loiter_Vahzilok_D10_V1, Loiter_Vahzilok_D10_V2,},
  },
}

--REQUIRED: This inserts universal into this map's OL
local UniOnce
if UniOnce == false or UniOnce == nil then
    InsertUniversals()
    UniOnce = true
end