--This is a rank table
--There could be multiple tables to generate spawns from
local FreaksVsFamily_Ranks_01  = {
        ["Freaks_Underlings"] = {
          --NA
        },
        ["Freaks_Minions"] = {
        "FRK_01", "FRK_02", "FRK_03",
        "FRK_04", "FRK_05", "FRK_06",
        "FRK_07",
        },
        ["Freaks_Lieutenants"] = {
        "FRK_21", "FRK_22", "FRK_23",
        },
        ["Freaks_Sniper"] = {
          --NA
        },
        ["Freaks_Boss"] = {
          --NA
        },
        ["Freaks_Elite Boss"] = {
          --NA
        },
        ["Victims"] = {
        },
        ["Freaks_Specials"] = {
        "FRK_24", "FRK_25", "FRK_26",
        "FRK_38", "FRK_41", "FRK_42",
        },
        ["Family_Minions"] = {
        "Thug_Family_01", "Thug_Family_02", "Thug_Family_03",
        "Thug_Family_04", "Thug_Family_05", "Thug_Family_06",
        },
        ["Family_Lieutenants"] = {
        "Thug_Family_Boss_01", "Thug_Family_Boss_02"
        },
        ["Family_Sniper"] = {
          --NA
        },
        ["Family_Boss"] = {
         --NA
        },
        ["Family_Elite Boss"] = {
          --NA
        },
        ["Victims"] = {
        },
        ["Family_Specials"] = {
        "Thug_Family_01", "Thug_Family_02", "Thug_Family_03",
        "Thug_Family_04", "Thug_Family_05", "Thug_Family_06",
        },
}

--[[
        These are the spawndefs.
]]


Rumble_FreakshowFamily_D5_V0= {
        ["RumbleCatwalk"] = {
                ["Encounter_S_30"] = FreaksVsFamily_Ranks_01.Family_Lieutenants,
                ["Encounter_E_02"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_E_04"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_S_31"] = FreaksVsFamily_Ranks_01.Freaks_Lieutenants,
                ["Encounter_E_09"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
                ["Encounter_E_05"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
        },
}

Rumble_FreakshowFamily_D5_V1= {
        ["RumbleCatwalk"] = {
                ["Encounter_S_30"] = FreaksVsFamily_Ranks_01.Family_Lieutenants,
                ["Encounter_E_04"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_E_10"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_E_06"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_S_33"] = FreaksVsFamily_Ranks_01.Freaks_Specials,
                ["Encounter_S_31"] = FreaksVsFamily_Ranks_01.Freaks_Lieutenants,
                ["Encounter_E_03"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
                ["Encounter_E_11"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
        },
}

Rumble_FreakshowFamily_D5_V2= {
        ["RumbleCatwalk"] = {
                ["Encounter_S_30"] = FreaksVsFamily_Ranks_01.Family_Lieutenants,
                ["Encounter_S_32"] = FreaksVsFamily_Ranks_01.Family_Lieutenants,
                ["Encounter_E_02"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_E_10"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_E_06"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_S_33"] = FreaksVsFamily_Ranks_01.Freaks_Specials,
                ["Encounter_S_31"] = FreaksVsFamily_Ranks_01.Freaks_Lieutenants,
                ["Encounter_S_35"] = FreaksVsFamily_Ranks_01.Freaks_Lieutenants,
                ["Encounter_E_03"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
                ["Encounter_E_11"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
        },
}

Rumble_FreakshowFamily_D5_V3 = {
        ["RumbleCatwalk"] = {
                ["Encounter_S_30"] = FreaksVsFamily_Ranks_01.Family_Lieutenants,
                ["Encounter_S_32"] = FreaksVsFamily_Ranks_01.Family_Lieutenants,
                ["Encounter_S_34"] = FreaksVsFamily_Ranks_01.Family_Lieutenants,
                ["Encounter_E_02"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_E_10"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_E_12"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_E_06"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_S_33"] = FreaksVsFamily_Ranks_01.Freaks_Specials,
                ["Encounter_S_31"] = FreaksVsFamily_Ranks_01.Freaks_Lieutenants,
                ["Encounter_S_35"] = FreaksVsFamily_Ranks_01.Freaks_Lieutenants,
                ["Encounter_E_09"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
                ["Encounter_E_01"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
                ["Encounter_E_03"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
                ["Encounter_E_11"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
        },
}

Rumble_FreakshowFamily_D5_V4 = {
        ["RumbleCatwalk"] = {
                ["Encounter_S_30"] = FreaksVsFamily_Ranks_01.Family_Lieutenants,
                ["Encounter_S_32"] = FreaksVsFamily_Ranks_01.Family_Lieutenants,
                ["Encounter_S_34"] = FreaksVsFamily_Ranks_01.Family_Lieutenants,
                ["Encounter_E_02"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_E_10"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_E_12"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_E_08"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_E_06"] = FreaksVsFamily_Ranks_01.Family_Minions,
                ["Encounter_S_33"] = FreaksVsFamily_Ranks_01.Freaks_Specials,
                ["Encounter_S_31"] = FreaksVsFamily_Ranks_01.Freaks_Lieutenants,
                ["Encounter_S_35"] = FreaksVsFamily_Ranks_01.Freaks_Lieutenants,
                ["Encounter_E_09"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
                ["Encounter_E_01"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
                ["Encounter_E_03"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
                ["Encounter_E_11"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
                ["Encounter_E_05"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
                ["Encounter_E_07"] = FreaksVsFamily_Ranks_01.Freaks_Minions,
        },
}
