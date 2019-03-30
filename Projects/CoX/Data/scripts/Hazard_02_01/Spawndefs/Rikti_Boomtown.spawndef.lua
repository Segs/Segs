--This is a rank table
--There could be multiple tables to generate spawns from
local Rikti_Ranks_01  = {
    ["Minions"] = {
        "Lost_01", "Lost_02", "Lost_03",
        "Lost_04", "Lost_05", "Lost_06",
        "Lost_07", "Lost_08", "Lost_09",
        "Lost_10", "Lost_11", "Lost_12",
    },
    ["Lieutenants"] = {
        "Lost_20", "Lost_21", "Lost_22",
        "Lost_23", "Lost_24", "Lost_25",
    },
    ["Boss"] = {
        "Lost_40", "Lost_41", "Lost_42",
        "Lost_43", "Lost_44",
    }
}


Preaching_RiktiBeggars_L14_17_V0 = {
    ["Markers"] = {
            ["Encounter_V_40"] = Rikti_Ranks_01.Boss,
            ["Encounter_E_02"] = Rikti_Ranks_01.Minions,
            ["Encounter_E_07"] = Rikti_Ranks_01.Minions,
            ["Encounter_E_06"] = Rikti_Ranks_01.Lieutenants,             
    },
}

Preaching_RiktiBeggars_L18_20_V0 = {
    ["Markers"] = {
            ["Encounter_V_40"] = Rikti_Ranks_01.Boss,
            ["Encounter_E_02"] = Rikti_Ranks_01.Minions,
            ["Encounter_E_07"] = Rikti_Ranks_01.Minions,
            ["Encounter_E_04"] = Rikti_Ranks_01.Minions,
            ["Encounter_E_07"] = Rikti_Ranks_01.Minions,             
    },
}