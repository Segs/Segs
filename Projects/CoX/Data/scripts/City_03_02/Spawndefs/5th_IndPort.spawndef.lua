--This is a rank table
--There could be multiple tables to generate spawns from
local Fifth_Ranks_01  = {
    ["Minions"] = {
        "5thNight_00", "5thNight_01",
    },
    ["Lieutenants"] = {
        "5thUber_40",
    },
}

--== RECRUIT ==--

Recruit_5thColumn_D1_V0 = {
    ["Markers"] = {
            ["Encounter_S_31"] = Fifth_Ranks_01.Lieutenants,
            ["Encounter_E_11"] = Fifth_Ranks_01.Minions,
   },
}

Recruit_5thColumn_D1_V1 = {
    ["Markers"] = {
            ["Encounter_S_31"] = Fifth_Ranks_01.Lieutenants,
            ["Encounter_E_11"] = Fifth_Ranks_01.Minions,
            ["Encounter_E_07"] = Fifth_Ranks_01.Minions,            
            ["Encounter_E_01"] = Fifth_Ranks_01.Minions,
   },
}

Recruit_5thColumn_D1_V2 = {
    ["Markers"] = {
            ["Encounter_S_31"] = Fifth_Ranks_01.Lieutenants,
            ["Encounter_E_11"] = Fifth_Ranks_01.Minions,
            ["Encounter_E_05"] = Fifth_Ranks_01.Minions,            
            ["Encounter_E_03"] = Fifth_Ranks_01.Minions,
   },
}

Recruit_5thColumn_D1_V3 = {
    ["Markers"] = {
            ["Encounter_S_31"] = Fifth_Ranks_01.Lieutenants,
            ["Encounter_E_11"] = Fifth_Ranks_01.Minions,
            ["Encounter_E_07"] = Fifth_Ranks_01.Minions,            
            ["Encounter_E_01"] = Fifth_Ranks_01.Minions,          
            ["Encounter_E_05"] = Fifth_Ranks_01.Minions,
            ["Encounter_E_09"] = Fifth_Ranks_01.Minions,
   },
}

Recruit_5thColumn_D1_V4 = {
    ["Markers"] = {
        ["Encounter_S_31"] = Fifth_Ranks_01.Lieutenants,
        ["Encounter_S_30"] = Fifth_Ranks_01.Lieutenants,
        ["Encounter_E_11"] = Fifth_Ranks_01.Minions,
        ["Encounter_E_07"] = Fifth_Ranks_01.Minions,            
        ["Encounter_E_01"] = Fifth_Ranks_01.Minions,          
        ["Encounter_E_05"] = Fifth_Ranks_01.Minions,
        ["Encounter_E_09"] = Fifth_Ranks_01.Minions,          
        ["Encounter_E_08"] = Fifth_Ranks_01.Minions,
        ["Encounter_E_04"] = Fifth_Ranks_01.Minions,
   },
}

Recruit_5thColumn_D5_V0 = Recruit_5thColumn_D1_V0
Recruit_5thColumn_D5_V1 = Recruit_5thColumn_D1_V1
Recruit_5thColumn_D5_V2 = Recruit_5thColumn_D1_V2
Recruit_5thColumn_D5_V3 = Recruit_5thColumn_D1_V3
Recruit_5thColumn_D5_V4 = Recruit_5thColumn_D1_V4

Recruit_5thColumn_D10_V0 = Recruit_5thColumn_D1_V0
Recruit_5thColumn_D10_V1 = Recruit_5thColumn_D1_V1
Recruit_5thColumn_D10_V2 = Recruit_5thColumn_D1_V2
Recruit_5thColumn_D10_V3 = Recruit_5thColumn_D1_V3
Recruit_5thColumn_D10_V4 = Recruit_5thColumn_D1_V4