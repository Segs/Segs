--Steel Canyon
include_lua('./Spawndefs/Outcasts_Steel.spawndef.lua')
include_lua('./Spawndefs/Clockwork_Steel.spawndef.lua')
include_lua('./Spawndefs/Tsoo_Steel.spawndef.lua')
include_lua('./Spawndefs/CircleOfThorns_Steel.spawndef.lua')
include_lua('./Spawndefs/5th_Steel.spawndef.lua')
include_lua('./Spawndefs/Vahzilok_Steel.spawndef.lua')

ES_Library_Objects = {
    ["ES_Outcasts_D1_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6","CanSpawn7"},
        ["CanSpawnDefs"] = {Mugging_Outcasts_D1_V0, Mugging_Outcasts_D1_V1, Mugging_Outcasts_D1_V2, Mugging_Outcasts_D1_V3,
                            ShadyDeal_Outcasts_D1_V0, ShadyDeal_Outcasts_D1_V1, ShadyDeal_Outcasts_D1_V2,},
    },
    ["ES_Outcasts_D5_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6","CanSpawn7"},
        ["CanSpawnDefs"] = {Mugging_Outcasts_D5_V0, Mugging_Outcasts_D5_V1, Mugging_Outcasts_D5_V2, Mugging_Outcasts_D5_V3,
                            ShadyDeal_Outcasts_D5_V0, ShadyDeal_Outcasts_D5_V1, ShadyDeal_Outcasts_D5_V2,},
    },
    ["ES_Outcasts_D9_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6","CanSpawn7"},
        ["CanSpawnDefs"] = {Mugging_Outcasts_D9_V0, Mugging_Outcasts_D9_V1, Mugging_Outcasts_D9_V2, Mugging_Outcasts_D9_V3,
                            ShadyDeal_Outcasts_D9_V0, ShadyDeal_Outcasts_D9_V1, ShadyDeal_Outcasts_D9_V2,},
    },
    ["ES_Outcasts_D3_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6",},
        ["CanSpawnDefs"] = {FightClub_Outcasts_D3_V0, FightClub_Outcasts_D3_V1, FightClub_Outcasts_D3_V2,
                            Loiter_Outcasts_D3_V0, Loiter_Outcasts_D3_V1, Loiter_Outcasts_D3_V2,},
    },
    ["ES_Outcasts_D8_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6",},
        ["CanSpawnDefs"] = {FightClub_Outcasts_D8_V0, FightClub_Outcasts_D8_V1, FightClub_Outcasts_D8_V2,
                            Loiter_Outcasts_D8_V0, Loiter_Outcasts_D8_V1, Loiter_Outcasts_D8_V2,},
    },
    ["ES_Outcasts_D1_PurseSnatch_City_02_01"] = {
        ["EncounterSpawn"] = "Snatch",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {PurseSnatch_Outcasts_D1_V0, PurseSnatch_Outcasts_D1_V1, PurseSnatch_Outcasts_D1_V2,},
      },
      ["ES_Outcasts_D5_PurseSnatch_City_02_01"] = {
        ["EncounterSpawn"] = "Snatch",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {PurseSnatch_Outcasts_D5_V0, PurseSnatch_Outcasts_D5_V1, PurseSnatch_Outcasts_D5_V2,},
      },
      ["ES_Outcasts_D9_PurseSnatch_City_02_01"] = {
        ["EncounterSpawn"] = "Snatch",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {PurseSnatch_Outcasts_D9_V0, PurseSnatch_Outcasts_D9_V1, PurseSnatch_Outcasts_D9_V2,},
      },
      ["ES_Outcasts_D3_Vandalism_City_02_01"] = {
        ["EncounterSpawn"] = "AroundVandalism",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Vandalism_Outcasts_D3_V0, Vandalism_Outcasts_D3_V1, Vandalism_Outcasts_D3_V2,},
      },
      ["ES_Outcasts_D8_Vandalism_City_02_01"] = {
        ["EncounterSpawn"] = "AroundVandalism",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Vandalism_Outcasts_D8_V0, Vandalism_Outcasts_D8_V1, Vandalism_Outcasts_D8_V2,},
      },
      ["ES_Outcasts_D3_AroundDoor_City_02_01"] = {
        ["EncounterSpawn"] = "AroundDoor",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {BreakingIn_Outcasts_D3_V0, BreakingIn_Outcasts_D3_V1, BreakingIn_Outcasts_D3_V2},
      },
      ["ES_Outcasts_D8_AroundDoor_City_02_01"] = {
         ["EncounterSpawn"] = "AroundDoor",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {BreakingIn_Outcasts_D8_V0, BreakingIn_Outcasts_D8_V1, BreakingIn_Outcasts_D8_V2},
      },
      ["ES_Clockwork_D1_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {Scavenge_Clockwork_D1_V0, Scavenge_Clockwork_D1_V1, Scavenge_Clockwork_D1_V2,
                            Scavenge_Clockwork_D1_V3,},
    },
    ["ES_Clockwork_D5_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {Scavenge_Clockwork_D5_V0, Scavenge_Clockwork_D5_V1, Scavenge_Clockwork_D5_V2, Scavenge_Clockwork_D5_V3,},
    },
    ["ES_Clockwork_D9_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {Scavenge_Clockwork_D9_V0, Scavenge_Clockwork_D9_V1, Scavenge_Clockwork_D9_V2, Scavenge_Clockwork_D9_V3,},
    },
    ["ES_TsooOutcasts_D3_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Threaten_TsooOutcasts_D3_V0, Threaten_TsooOutcasts_D3_V1, Threaten_TsooOutcasts_D3_V2,},
    },
    ["ES_TsooOutcasts_D8_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Threaten_TsooOutcasts_D8_V0, Threaten_TsooOutcasts_D8_V1, Threaten_TsooOutcasts_D8_V2,},
    },
    ["ES_CircleOfThorns_D3_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Sacrifice_CoT_D3_V0, Sacrifice_CoT_D3_V1, Sacrifice_CoT_D3_V2, Sacrifice_CoT_D3_V3, Sacrifice_CoT_D3_V4,},
    },
    ["ES_CircleOfThorns_D8_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Sacrifice_CoT_D8_V0, Sacrifice_CoT_D8_V1, Sacrifice_CoT_D8_V2, Sacrifice_CoT_D8_V3, Sacrifice_CoT_D8_V4,},
    },
    ["ES_5thColumn_D3_GroupedDirection_City_02_01"] = {
        ["EncounterSpawn"] = "GroupedDirection",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Proselytize_5thColumn_D3_V0, Proselytize_5thColumn_D3_V1, Proselytize_5thColumn_D3_V2,},
    },
    ["ES_5thColumn_D8_GroupedDirection_City_02_01"] = {
        ["EncounterSpawn"] = "GroupedDirectionw",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Proselytize_5thColumn_D8_V0, Proselytize_5thColumn_D8_V1, Proselytize_5thColumn_D8_V2,},
    },
    ["ES_Vahzilok_D3_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Hunting_Vahzilok_D3_V0, Hunting_Vahzilok_D3_V1, Hunting_Vahzilok_D3_V2,},
    },
    ["ES_Vahzilok_D8_Around_City_02_01"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Hunting_Vahzilok_D8_V0, Hunting_Vahzilok_D8_V1, Hunting_Vahzilok_D8_V2,},
    },
    ["ES_TrollsOutcasts_D1_Rumble_City_02_01"] = {
        ["EncounterSpawn"] = "Rumble",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Rumble_OutcastsTrolls_D1_V0, Rumble_OutcastsTrolls_D1_V1, Rumble_OutcastsTrolls_D1_V2,},
    },
    ["ES_TrollsOutcasts_D5_Rumble_City_02_01"] = {
        ["EncounterSpawn"] = "Rumble",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Rumble_OutcastsTrolls_D5_V0, Rumble_OutcastsTrolls_D5_V1, Rumble_OutcastsTrolls_D5_V2,},
    },
    ["ES_FamilyOutcasts_D3_Rumble_City_02_01"] = {
        ["EncounterSpawn"] = "Rumble",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Negotiation_OutcastsFamily_D3_V0, Negotiation_OutcastsFamily_D3_V1, Negotiation_OutcastsFamily_D3_V2,},
    },
    ["ES_FamilyOutcasts_D8_Rumble_City_02_01"] = {
        ["EncounterSpawn"] = "Rumble",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Negotiation_OutcastsFamily_D8_V0, Negotiation_OutcastsFamily_D8_V1, Negotiation_OutcastsFamily_D8_V2,},
    },    
}

--REQUIRED: This inserts universal into this map's OL
local UniOnce
if UniOnce == false or UniOnce == nil then
    InsertUniversals()
    UniOnce = true
end