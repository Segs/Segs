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

--== PROSELYTIZE ==--

Proselytize_5thColumn_D3_V0 = {
    ["Markers"] = {
            ["Encounter_S_31"] = Fifth_Ranks_01.Lieutenants,
            ["Encounter_E_11"] = Fifth_Ranks_01.Minions,
            ["Encounter_E_07"] = Fifth_Ranks_01.Minions,            
            ["Encounter_E_01"] = Fifth_Ranks_01.Minions,
   },
}

Proselytize_5thColumn_D3_V1 = {
    ["Markers"] = {
            ["Encounter_S_31"] = Fifth_Ranks_01.Lieutenants,
            ["Encounter_E_11"] = Fifth_Ranks_01.Minions,
            ["Encounter_E_07"] = Fifth_Ranks_01.Minions,            
            ["Encounter_E_01"] = Fifth_Ranks_01.Minions,          
            ["Encounter_E_05"] = Fifth_Ranks_01.Minions,
            ["Encounter_E_09"] = Fifth_Ranks_01.Minions,
   },
}

Proselytize_5thColumn_D3_V2 = {
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

Proselytize_5thColumn_D8_V0 = Proselytize_5thColumn_D3_V0
Proselytize_5thColumn_D8_V1 = Proselytize_5thColumn_D3_V1
Proselytize_5thColumn_D8_V2 = Proselytize_5thColumn_D3_V2
