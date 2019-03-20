--This is a rank table
--There could be multiple tables to generate spawns from
local Hellions_Ranks_01  = {
        ["Underlings"] = {
          --NA
        },
        ["Minions"] = {
                "Thug_Hellion_01", "Thug_Hellion_02", "Thug_Hellion_03",
                "Thug_Hellion_04", "Thug_Hellion_05", "Thug_Hellion_06",
        },
        ["Lieutenants"] = {
                "Thug_Hellion_01", "Thug_Hellion_02", "Thug_Hellion_03",
                "Thug_Hellion_04", "Thug_Hellion_05", "Thug_Hellion_06",
        },
        ["Sniper"] = {
          --NA
        },
        ["Boss"] = {
                "Thug_Hellion_Boss_01", "Thug_Hellion_Boss_02", "Thug_Hellion_Boss_03",
        },
        ["Elite Boss"] = {
          --NA
        },
        ["Victims"] = {

        },
        ["Specials"] = {
                "Thug_Hellion_01", "Thug_Hellion_02", "Thug_Hellion_03",
                "Thug_Hellion_04", "Thug_Hellion_05", "Thug_Hellion_06",
        },
}

--[[
        These are the spawndefs.
]]


Loiter_Hellions_L1_3_V0 = {
        ["Markers"] = {
            ["Encounter_S_30"] = Hellions_Ranks_01.Lieutenants,
            ["Encounter_E_02"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_03"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_07"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_08"] = Hellions_Ranks_01.Minions,
       },
}

Loiter_Hellions_L1_3_V1 = Loiter_Hellions_L1_3_V0
Loiter_Hellions_L1_3_V2 = Loiter_Hellions_L1_3_V0
Loiter_Hellions_L1_3_V3 = Loiter_Hellions_L1_3_V0
Loiter_Hellions_L1_3_V4 = Loiter_Hellions_L1_3_V0

Loiter_Hellions_L4_7_V0 = {
        ["Markers"] = {
            ["Encounter_S_30"] = Hellions_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Hellions_Ranks_01.Lieutenants,
            ["Encounter_E_01"] = Hellions_Ranks_01.Minions,            
            ["Encounter_E_03"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_07"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_08"] = Hellions_Ranks_01.Minions,
       },
}

Loiter_Hellions_L4_7_V1 = Loiter_Hellions_L4_7_V0
Loiter_Hellions_L4_7_V2 = Loiter_Hellions_L4_7_V0
Loiter_Hellions_L4_7_V3 = Loiter_Hellions_L4_7_V0
Loiter_Hellions_L4_7_V4 = Loiter_Hellions_L4_7_V0

Loiter_Hellions_L8_10_V0 = {
        ["Markers"] = {
            ["Encounter_S_30"] = Hellions_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Hellions_Ranks_01.Boss,            
            ["Encounter_S_32"] = Hellions_Ranks_01.Lieutenants,
            ["Encounter_E_01"] = Hellions_Ranks_01.Minions,            
            ["Encounter_E_02"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_03"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_04"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_05"] = Hellions_Ranks_01.Minions,            
            ["Encounter_E_06"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_07"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_08"] = Hellions_Ranks_01.Minions,            
       },
}


Rumble_HellionsSkulls_L4_7_V0 = {
        ["Markers"] = {
            ["Encounter_S_30"] = Hellions_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Hellions_Ranks_01.Lieutenants,
            ["Encounter_E_01"] = Hellions_Ranks_01.Minions,            
            ["Encounter_E_03"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_07"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_08"] = Hellions_Ranks_01.Minions,
       },
}

Rumble_HellionsSkulls_L4_7_V1 = Rumble_HellionsSkulls_L4_7_V0
Rumble_HellionsSkulls_L4_7_V2 = Rumble_HellionsSkulls_L4_7_V0


Rumble_HellionsSkulls_L8_10_V0 = Rumble_HellionsSkulls_L4_7_V0
Rumble_HellionsSkulls_L8_10_V1 = Rumble_HellionsSkulls_L4_7_V0
Rumble_HellionsSkulls_L8_10_V2 = Rumble_HellionsSkulls_L4_7_V0

