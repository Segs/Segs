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

--== Training ==--

Training_5thCol_L12_15_V0 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Fifth_Ranks_01.Lieutenants,
        ["Encounter_S_31"] = Fifth_Ranks_01.Lieutenants,
        ["Encounter_E_01"] = Fifth_Ranks_01.Minions,
        ["Encounter_E_02"] = Fifth_Ranks_01.Minions,
        ["Encounter_E_03"] = Fifth_Ranks_01.Minions,
        ["Encounter_E_06"] = Fifth_Ranks_01.Minions,            
   },
}
