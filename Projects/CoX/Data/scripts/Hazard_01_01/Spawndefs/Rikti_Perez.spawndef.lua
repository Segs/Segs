--This is a rank table
--There could be multiple tables to generate spawns from
local Rikti_Ranks_01  = {
        ["Underlings"] = {
          --NA
        },
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
        ["Sniper"] = {
          --NA
        },
        ["Boss"] = {

        },
        ["Elite Boss"] = {

        },
        ["Victims"] = {

        },
        ["Specials"] = {

        },
}

--[[
        These are the spawndefs.
]]


Preaching_RiktiBeggar_L8_10_V0 = {
        ["Markers"] = {
                ["Encounter_E_05"] = Rikti_Ranks_01.Minions,
                ["Encounter_E_07"] = Rikti_Ranks_01.Lieutenants,
        },
}

Preaching_RiktiBeggar_L8_10_V1 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Rikti_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Rikti_Ranks_01.Minions,
                ["Encounter_E_07"] = Rikti_Ranks_01.Minions,
                ["Encounter_E_01"] = Rikti_Ranks_01.Lieutenants,                
        },
}

Preaching_RiktiBeggar_L8_10_V2 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Rikti_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Rikti_Ranks_01.Minions,
                ["Encounter_E_07"] = Rikti_Ranks_01.Minions,
                ["Encounter_E_01"] = Rikti_Ranks_01.Lieutenants,              
        },
}
