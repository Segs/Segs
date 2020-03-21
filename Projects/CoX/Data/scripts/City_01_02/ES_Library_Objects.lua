--King's Row
include_lua('./Spawndefs/Skulls_Kings.spawndef.lua')
include_lua('./Spawndefs/Vahzilok_Kings.spawndef.lua')
include_lua('./Spawndefs/CircleOfThorns_Kings.spawndef.lua')
include_lua('./Spawndefs/Clockwork_Kings.spawndef.lua')
include_lua('./Spawndefs/Rikti_Kings.spawndef.lua')

ES_Library_Objects = {
    ["ES_Skulls_D01_Around_City_01_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6","CanSpawn7"},
        ["CanSpawnDefs"] = {Skulls_Mugging_D1_V0, Skulls_Mugging_D1_V1, Skulls_Mugging_D1_V2, 
                            Skulls_Mugging_D1_V3, Skulls_ShadyDeal_D1_V0, 
                            Skulls_ShadyDeal_D1_V1, Skulls_ShadyDeal_D1_V2},
    },
    ["ES_Skulls_D05_Around_City_01_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6","CanSpawn7"},
        ["CanSpawnDefs"] = {Skulls_Mugging_D5_V0, Skulls_Mugging_D5_V1, Skulls_Mugging_D5_V2, 
                            Skulls_Mugging_D5_V3, Skulls_ShadyDeal_D5_V0, 
                            Skulls_ShadyDeal_D5_V1, Skulls_ShadyDeal_D5_V2},
    },
    ["ES_Skulls_D10_Around_City_01_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6","CanSpawn7","CanSpawn8","CanSpawn9","CanSpawn10"},
        ["CanSpawnDefs"] = {Skulls_Mugging_D10_V0, Skulls_Mugging_D10_V1, Skulls_Mugging_D10_V2, 
                            Skulls_Mugging_D10_V3, Skulls_ShadyDeal_D10_V0, Skulls_ShadyDeal_D10_V1, 
                            Skulls_ShadyDeal_D10_V2, SkullsTrolls_Deal_D10_V0, SkullsTrolls_Deal_D10_V1, 
                            SkullsTrolls_Deal_D10_V2},
    },
    ["ES_Skulls_D01_PurseSnatch_City_01_02"] = {
        ["EncounterSpawn"] = "Snatch",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Skulls_PurseSnatch_D1_V0, Skulls_PurseSnatch_D1_V1, Skulls_PurseSnatch_D1_V2,},
    },
    ["ES_Skulls_D05_PurseSnatch_City_01_02"] = {
        ["EncounterSpawn"] = "Snatch",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Skulls_PurseSnatch_D5_V0, Skulls_PurseSnatch_D5_V1, Skulls_PurseSnatch_D5_V2,},
    },
    ["ES_Skulls_D10_PurseSnatch_City_01_02"] = {
        ["EncounterSpawn"] = "Snatch",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Skulls_PurseSnatch_D10_V0, Skulls_PurseSnatch_D10_V1, Skulls_PurseSnatch_D10_V2,},
    },
    ["ES_Skulls_D01_Vandalism_City_01_02"] = {
        ["EncounterSpawn"] = "AroundVandalism",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Skulls_Vandalism_D1_V0, Skulls_Vandalism_D1_V1, Skulls_Vandalism_D1_V1}, --double V1 is not a mistake?
    },
    ["ES_Skulls_D05_Vandalism_City_01_02"] = {
        ["EncounterSpawn"] = "AroundVandalism",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Skulls_Vandalism_D5_V0, Skulls_Vandalism_D5_V1, Skulls_Vandalism_D5_V1}, --double V1 is not a mistake?
    },
    ["ES_Skulls_D10_Vandalism_City_01_02"] = {
        ["EncounterSpawn"] = "AroundVandalism",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Skulls_Vandalism_D10_V0, Skulls_Vandalism_D10_V1, Skulls_Vandalism_D10_V1}, --double V1 is not a mistake?
    },
    ["ES_Skulls_D01_AroundDoor_City_01_02"] = {
        ["EncounterSpawn"] = "AroundVandalism",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Skulls_BreakIn_D1_V0, Skulls_BreakIn_D1_V1, Skulls_BreakIn_D1_V2}, --had to change name from break-in due to Lua
    },
    ["ES_Skulls_D05_AroundDoor_City_01_02"] = {
        ["EncounterSpawn"] = "AroundVandalism",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Skulls_BreakIn_D5_V0, Skulls_BreakIn_D5_V1, Skulls_BreakIn_D5_V2}, --had to change name from break-in due to Lua
    },
    ["ES_Skulls_D10_AroundDoor_City_01_02"] = {
        ["EncounterSpawn"] = "AroundVandalism",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Skulls_BreakIn_D10_V0, Skulls_BreakIn_D10_V1, Skulls_BreakIn_D10_V2}, --had to change name from break-in due to Lua
    },
    ["ES_SkullsFight_D03_Around_City_01_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6",},
        ["CanSpawnDefs"] = {Skulls_FightClub_D3_V0, Skulls_FightClub_D3_V1, Skulls_FightClub_D3_V2, 
                            Skulls_BeatDown_D3_V0, Skulls_BeatDown_D3_V1, Skulls_BeatDown_D3_V2},
    },
    ["ES_SkullsFight_D08_Around_City_01_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6",},
        ["CanSpawnDefs"] = {Skulls_FightClub_D8_V0, Skulls_FightClub_D8_V1, Skulls_FightClub_D8_V2, 
                            Skulls_BeatDown_D8_V0, Skulls_BeatDown_D8_V1, Skulls_BeatDown_D8_V2},
    },
    ["ES_SkullsAmbush_D01_Ambush_City_01_02_1"] = {
        ["EncounterSpawn"] = "Ambush",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Skulls_Ambush_D1_V0, Skulls_Ambush_D1_V1, Skulls_Ambush_D1_V2},
    },     
    ["ES_SkullsAmbush_D05_Ambush_City_01_02_1"] = {
        ["EncounterSpawn"] = "Ambush",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Skulls_Ambush_D5_V0, Skulls_Ambush_D5_V1, Skulls_Ambush_D5_V2},
    },
    ["ES_SkullsAmbush_D10_Ambush_City_01_02_1"] = {
        ["EncounterSpawn"] = "Ambush",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Skulls_Ambush_D10_V0, Skulls_Ambush_D10_V1, Skulls_Ambush_D10_V2},
    },
    ["ES_SkullsVahzilok_D01_Rumble_City_01_02"] = {
        ["EncounterSpawn"] = "Rumble",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {SkullsVahzilok_Rumble_D1_V0, SkullsVahzilok_Rumble_D1_V1, 
                            SkullsVahzilok_Rumble_D1_V2, SkullsVahzilok_Rumble_D1_V3},
    },
    ["ES_SkullsVahzilok_D05_Rumble_City_01_02"] = {
        ["EncounterSpawn"] = "Rumble",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {SkullsVahzilok_Rumble_D5_V0, SkullsVahzilok_Rumble_D5_V1, 
                            SkullsVahzilok_Rumble_D5_V2, SkullsVahzilok_Rumble_D5_V3},
    },
    ["ES_SkullsVahzilok_D10_Rumble_City_01_02"] = {
        ["EncounterSpawn"] = "Rumble",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {SkullsVahzilok_Rumble_D10_V0, SkullsVahzilok_Rumble_D10_V1, 
                            SkullsVahzilok_Rumble_D10_V2, SkullsVahzilok_Rumble_D10_V3},
    },
    ["ES_Vahzilok_D03_Around_City_01_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6",},
        ["CanSpawnDefs"] = {Vahzilok_Harvest_D3_V0, Vahzilok_Harvest_D3_V1, Vahzilok_Harvest_D3_V2,
                        Vahzilok_Hunting_D3_V0, Vahzilok_Hunting_D3_V1, Vahzilok_Hunting_D3_V2,},
    },
    ["ES_Vahzilok_D08_Around_City_01_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6",},
        ["CanSpawnDefs"] = {Vahzilok_Harvest_D8_V0, Vahzilok_Harvest_D8_V1, Vahzilok_Harvest_D8_V2,
                        Vahzilok_Hunting_D8_V0, Vahzilok_Hunting_D8_V1, Vahzilok_Hunting_D8_V2,},
    },
    ["ES_VahzilokAmbush_D01_Ambush_City_01_02_1"] = {
        ["EncounterSpawn"] = "Ambush",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Vahzilok_Ambush_D1_V0, Vahzilok_Ambush_D1_V1, Vahzilok_Ambush_D1_V2},
    },
    ["ES_VahzilokAmbush_D05_Ambush_City_01_02_1"] = {
        ["EncounterSpawn"] = "Ambush",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Vahzilok_Ambush_D5_V0, Vahzilok_Ambush_D5_V1, Vahzilok_Ambush_D5_V2},
    },
    ["ES_VahzilokAmbush_D10_Ambush_City_01_02_1"] = {
        ["EncounterSpawn"] = "Ambush",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {Vahzilok_Ambush_D10_V0, Vahzilok_Ambush_D10_V1, Vahzilok_Ambush_D10_V2},
    },
    ["ES_CircleOfThorns_D03_Around_City_01_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3",},
        ["CanSpawnDefs"] = {CircleOfThorns_Ceremony_D3_V0, CircleOfThorns_Ceremony_D3_V1, CircleOfThorns_Ceremony_D3_V2},
    },
    ["ES_CircleOfThorns_D08_Around_City_01_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3"},
        ["CanSpawnDefs"] = {CircleOfThorns_Ceremony_D8_V0, CircleOfThorns_Ceremony_D8_V1, CircleOfThorns_Ceremony_D8_V2},
    },
    ["ES_Clockwork_D01_Around_City_01_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6","CanSpawn7"},
        ["CanSpawnDefs"] = {Clockwork_Building_D1_V0, Clockwork_Building_D1_V1, Clockwork_Building_D1_V2, Clockwork_Building_D1_V3,
                            Clockwork_Scavenge_D1_V0, Clockwork_Scavenge_D1_V1, Clockwork_Scavenge_D1_V2},
    },

    ["ES_Clockwork_D05_Around_City_01_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6","CanSpawn7"},
        ["CanSpawnDefs"] = {Clockwork_Building_D5_V0, Clockwork_Building_D5_V1, Clockwork_Building_D5_V2, Clockwork_Building_D5_V3,
                            Clockwork_Scavenge_D5_V0, Clockwork_Scavenge_D5_V1, Clockwork_Scavenge_D5_V2},
    },
    ["ES_Clockwork_D10_Around_City_01_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6","CanSpawn7"},
        ["CanSpawnDefs"] = {Clockwork_Building_D10_V0, Clockwork_Building_D10_V1, Clockwork_Building_D10_V2, Clockwork_Building_D10_V3,
                            Clockwork_Scavenge_D10_V0, Clockwork_Scavenge_D10_V1, Clockwork_Scavenge_D10_V2},
    },
    ["ES_Rikti_D05_Around_City_01_02"] = {
        ["EncounterSpawn"] = "AroundVandalism",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {Lost_Preaching_D5_V0, Lost_Preaching_D5_V1, 
                            Lost_Preaching_D5_V2, Lost_Preaching_D5_V3,},
    },
    ["ES_Rikti_D10_Around_City_01_02"] = {
        ["EncounterSpawn"] = "AroundVandalism",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {Lost_Preaching_D10_V0, Lost_Preaching_D10_V1, 
                            Lost_Preaching_D10_V2, Lost_Preaching_D10_V3,},
    },        
}

--REQUIRED: This inserts universal into this map's OL
local UniOnce
if UniOnce == false or UniOnce == nil then
    InsertUniversals()
    UniOnce = true
end