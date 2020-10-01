--Independence Port
include_lua('./Spawndefs/Family_IndPort.spawndef.lua')
include_lua('./Spawndefs/Tsoo_IndPort.spawndef.lua')
include_lua('./Spawndefs/Devouring_IndPort.spawndef.lua')
include_lua('./Spawndefs/5th_IndPort.spawndef.lua')

ES_Library_Objects = {
    ["ES_Family_D1_Around_City_03_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6",
                        "CanSpawn7", "CanSpawn8","CanSpawn9","CanSpawn10","CanSpawn11","CanSpawn12",
                        "CanSpawn13", "CanSpawn14","CanSpawn15","CanSpawn16","CanSpawn17",},
        ["CanSpawnDefs"] = {FightClub_Family_D1_V0, FightClub_Family_D1_V1, FightClub_Family_D1_V2, FightClub_Family_D1_V3,
                            Loiter_Family_D1_V0, Loiter_Family_D1_V1, Loiter_Family_D1_V2, Loiter_Family_D1_V3,
                            ShadyDeal_Family_D1_V0, ShadyDeal_Family_D1_V1, ShadyDeal_Family_D1_V2, ShadyDeal_Family_D1_V3,
                            Shakedown_Family_D1_V0, Shakedown_Family_D1_V1, Shakedown_Family_D1_V2, Shakedown_Family_D1_V3, Shakedown_Family_D1_V4,},
    },
    ["ES_Family_D5_Around_City_03_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6",
                        "CanSpawn7", "CanSpawn8","CanSpawn9","CanSpawn10","CanSpawn11","CanSpawn12",
                        "CanSpawn13", "CanSpawn14","CanSpawn15","CanSpawn16","CanSpawn17",},
        ["CanSpawnDefs"] = {FightClub_Family_D5_V0, FightClub_Family_D5_V1, FightClub_Family_D5_V2, FightClub_Family_D5_V3,
                            Loiter_Family_D5_V0, Loiter_Family_D5_V1, Loiter_Family_D5_V2, Loiter_Family_D5_V3,
                            ShadyDeal_Family_D5_V0, ShadyDeal_Family_D5_V1, ShadyDeal_Family_D5_V2, ShadyDeal_Family_D5_V3,
                            Shakedown_Family_D5_V0, Shakedown_Family_D5_V1, Shakedown_Family_D5_V2, Shakedown_Family_D5_V3, Shakedown_Family_D5_V4,},
    },
    ["ES_Family_D10_Around_City_03_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5","CanSpawn6",
                        "CanSpawn7", "CanSpawn8","CanSpawn9","CanSpawn10","CanSpawn11","CanSpawn12",
                        "CanSpawn13", "CanSpawn14","CanSpawn15","CanSpawn16","CanSpawn17",},
        ["CanSpawnDefs"] = {FightClub_Family_D10_V0, FightClub_Family_D10_V1, FightClub_Family_D10_V2, FightClub_Family_D10_V3,
                            Loiter_Family_D10_V0, Loiter_Family_D10_V1, Loiter_Family_D10_V2, Loiter_Family_D10_V3,
                            ShadyDeal_Family_D10_V0, ShadyDeal_Family_D10_V1, ShadyDeal_Family_D10_V2, ShadyDeal_Family_D10_V3,
                            Shakedown_Family_D10_V0, Shakedown_Family_D10_V1, Shakedown_Family_D10_V2, Shakedown_Family_D10_V3, Shakedown_Family_D10_V4,},
    },
    ["ES_FamilyTsoo_D3_Rumble_City_03_02"] = {
        ["EncounterSpawn"] = "Rumble",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Rumble_FamilyTsoo_D3_V0, Rumble_FamilyTsoo_D3_V1, Rumble_FamilyTsoo_D3_V2,
                            Rumble_FamilyTsoo_D3_V3, Rumble_FamilyTsoo_D3_V4,},
    },
    ["ES_FamilyTsoo_D8_Rumble_City_03_02"] = {
        ["EncounterSpawn"] = "Rumble",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Rumble_FamilyTsoo_D8_V0, Rumble_FamilyTsoo_D8_V1, Rumble_FamilyTsoo_D8_V2,
                            Rumble_FamilyTsoo_D8_V3, Rumble_FamilyTsoo_D8_V4,},
    },
    ["ES_5thTsoo_D3_Rumble_City_03_02"] = {
        ["EncounterSpawn"] = "Rumble",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Rumble_Tsoo5thColumn_D3_V0, Rumble_Tsoo5thColumn_D3_V1, Rumble_Tsoo5thColumn_D3_V2,
                            Rumble_Tsoo5thColumn_D3_V3, Rumble_Tsoo5thColumn_D3_V4,},
    },
    ["ES_5thTsoo_D8_Rumble_City_03_02"] = {
        ["EncounterSpawn"] = "Rumble",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Rumble_Tsoo5thColumn_D8_V0, Rumble_Tsoo5thColumn_D8_V1, Rumble_Tsoo5thColumn_D8_V2,
                            Rumble_Tsoo5thColumn_D8_V3, Rumble_Tsoo5thColumn_D8_V4,},
    },
    ["ES_Tsoo_D1_AroundVandalism_City_03_02"] = {
        ["EncounterSpawn"] = "AroundVandalism",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {Vandalism_Tsoo_D1_V0, Vandalism_Tsoo_D1_V1,
                            Vandalism_Tsoo_D1_V2, Vandalism_Tsoo_D1_V3,},
    },
    ["ES_Tsoo_D5_AroundVandalism_City_03_02"] = {
        ["EncounterSpawn"] = "AroundVandalism",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {Vandalism_Tsoo_D5_V0, Vandalism_Tsoo_D5_V1,
                            Vandalism_Tsoo_D5_V2, Vandalism_Tsoo_D5_V3,},
    },
    ["ES_Tsoo_D10_AroundVandalism_City_03_02"] = {
        ["EncounterSpawn"] = "AroundVandalism",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {Vandalism_Tsoo_D10_V0, Vandalism_Tsoo_D10_V1,
                            Vandalism_Tsoo_D10_V2, Vandalism_Tsoo_D10_V3,},
    },
    ["ES_Tsoo_D1_Around_City_03_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {Mugging_Tsoo_D1_V0, Mugging_Tsoo_D1_V1,
                            Mugging_Tsoo_D1_V2, Mugging_Tsoo_D1_V3,},
    },
    ["ES_Tsoo_D5_Around_City_03_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {Mugging_Tsoo_D5_V0, Mugging_Tsoo_D5_V1,
                            Mugging_Tsoo_D5_V2, Mugging_Tsoo_D5_V3,},
    },
    ["ES_Tsoo_D10_Around_City_03_02"] = {
        ["EncounterSpawn"] = "Around",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4",},
        ["CanSpawnDefs"] = {Mugging_Tsoo_D10_V0, Mugging_Tsoo_D10_V1,
                            Mugging_Tsoo_D10_V2, Mugging_Tsoo_D10_V3,},
    },
    ["ES_5thFamily_D3_Rumble_City_03_02"] = {
        ["EncounterSpawn"] = "Rumble",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Rumble_5thFamily_D3_V0, Rumble_5thFamily_D3_V1, Rumble_5thFamily_D3_V2,
                            Rumble_5thFamily_D3_V3, Rumble_5thFamily_D3_V4,},
    },
    ["ES_5thFamily_D8_Rumble_City_03_02"] = {
        ["EncounterSpawn"] = "Rumble",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Rumble_5thFamily_D8_V0, Rumble_5thFamily_D8_V1, Rumble_5thFamily_D8_V2,
                            Rumble_5thFamily_D8_V3, Rumble_5thFamily_D8_V4,},
    },
    ["ES_DE_Rampage_D5_City_03_02"] = {
        ["EncounterSpawn"] = "GroupedDirection",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Rampage_DevouringEarth_D5_V0, Rampage_DevouringEarth_D5_V1, Rampage_DevouringEarth_D5_V2, 
                           Rampage_DevouringEarth_D5_V3, Rampage_DevouringEarth_D5_V4,},
    },
    ["ES_FreaksVsFamily_D5_City_03_02"] = {
        ["EncounterSpawn"] = "RumbleCatwalk",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Rumble_FreakshowFamily_D5_V0, Rumble_FreakshowFamily_D5_V1, Rumble_FreakshowFamily_D5_V2,
                            Rumble_FreakshowFamily_D5_V3, Rumble_FreakshowFamily_D5_V4,},
    },
    ["ES_FreaksVsTsoo_D5_City_03_02"] = {
        ["EncounterSpawn"] = "RumbleCatwalk",
        ["CanSpawn"] = {"CanSpawn1", "CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Rumble_FreakshowTsoo_D5_V0, Rumble_FreakshowTsoo_D5_V1, Rumble_FreakshowTsoo_D5_V2,
                            Rumble_FreakshowTsoo_D5_V3, Rumble_FreakshowTsoo_D5_V4,},
    },
    ["ES_5thColumn_Recruit_D1_City_03_02"] = {
        ["EncounterSpawn"] = "GroupedDirection",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Recruit_5thColumn_D1_V0, Recruit_5thColumn_D1_V1, Recruit_5thColumn_D1_V2,
                            Recruit_5thColumn_D1_V3, Recruit_5thColumn_D1_V4},
    },
    ["ES_5thColumn_Recruit_D5_City_03_02"] = {
        ["EncounterSpawn"] = "GroupedDirection",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Recruit_5thColumn_D5_V0, Recruit_5thColumn_D5_V1, Recruit_5thColumn_D5_V2,
                            Recruit_5thColumn_D5_V3, Recruit_5thColumn_D5_V4},
    },
    ["ES_5thColumn_Recruit_D10_City_03_02"] = {
        ["EncounterSpawn"] = "GroupedDirection",
        ["CanSpawn"] = {"CanSpawn1","CanSpawn2","CanSpawn3","CanSpawn4","CanSpawn5",},
        ["CanSpawnDefs"] = {Recruit_5thColumn_D10_V0, Recruit_5thColumn_D10_V1, Recruit_5thColumn_D10_V2,
                            Recruit_5thColumn_D10_V3, Recruit_5thColumn_D10_V4},
    },                                  
}

--REQUIRED: This inserts universal into this map's OL
local UniOnce
if UniOnce == false or UniOnce == nil then
    InsertUniversals()
    UniOnce = true
end