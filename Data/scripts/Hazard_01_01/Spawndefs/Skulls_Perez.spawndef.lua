--This is a rank table
--There could be multiple tables to generate spawns from
local Skulls_Ranks_01  = {
        ["Underlings"] = {
          --NA
        },
        ["Minions"] = {
                "Thug_Skull_01", "Thug_Skull_02", "Thug_Skull_03",
                "Thug_Skull_04", "Thug_Skull_05", "Thug_Skull_06",
        },
        ["Lieutenants"] = {
                "Thug_Skull_01", "Thug_Skull_02", "Thug_Skull_03",
                "Thug_Skull_04", "Thug_Skull_05", "Thug_Skull_06",
        },
        ["Sniper"] = {
          --NA
        },
        ["Boss"] = {
                "Thug_Skull_Boss_01", "Thug_Skull_Boss_02", "Thug_Skull_Boss_03",
        },
        ["Elite Boss"] = {
          --NA
        },
        ["Victims"] = {

        },
        ["Specials"] = {
                "Thug_Skull_01", "Thug_Skull_02", "Thug_Skull_03",
                "Thug_Skull_04", "Thug_Skull_05", "Thug_Skull_06",
        },
}

--[[
        These are the spawndefs.
]]


Loiter_Skulls_L1_3_V0 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Skulls_Ranks_01.Minions,
                ["Encounter_S_31"] = Skulls_Ranks_01.Minions,
                ["Encounter_E_01"] = Skulls_Ranks_01.Minions,            
                ["Encounter_E_05"] = Skulls_Ranks_01.Minions,
                ["Encounter_E_04"] = Skulls_Ranks_01.Minions,
                ["Encounter_E_05"] = Skulls_Ranks_01.Minions,
       },
}

Loiter_Skulls_L1_3_V1 = Loiter_Skulls_L1_3_V0
Loiter_Skulls_L1_3_V2 = Loiter_Skulls_L1_3_V0
Loiter_Skulls_L1_3_V3 = Loiter_Skulls_L1_3_V0
Loiter_Skulls_L1_3_V4 = Loiter_Skulls_L1_3_V0

Loiter_Skulls_L4_7_V0 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Skulls_Ranks_01.Lieutenants,
                ["Encounter_S_31"] = Skulls_Ranks_01.Lieutenants,
                ["Encounter_E_01"] = Skulls_Ranks_01.Minions,            
                ["Encounter_E_05"] = Skulls_Ranks_01.Minions,
                ["Encounter_E_04"] = Skulls_Ranks_01.Minions,
                ["Encounter_E_05"] = Skulls_Ranks_01.Minions,
                ["Encounter_E_08"] = Skulls_Ranks_01.Minions,                
       },
}

Loiter_Skulls_L4_7_V1 = Loiter_Skulls_L4_7_V0
Loiter_Skulls_L4_7_V2 = Loiter_Skulls_L4_7_V0
Loiter_Skulls_L4_7_V3 = Loiter_Skulls_L4_7_V0
Loiter_Skulls_L4_7_V4 = Loiter_Skulls_L4_7_V0

Loiter_Skulls_L8_10_V0 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Skulls_Ranks_01.Lieutenants,
                ["Encounter_S_31"] = Skulls_Ranks_01.Boss,            
                ["Encounter_S_32"] = Skulls_Ranks_01.Lieutenants,
                ["Encounter_E_01"] = Skulls_Ranks_01.Minions,            
                ["Encounter_E_02"] = Skulls_Ranks_01.Minions,
                ["Encounter_E_03"] = Skulls_Ranks_01.Minions,
                ["Encounter_E_04"] = Skulls_Ranks_01.Minions,
                ["Encounter_E_05"] = Skulls_Ranks_01.Minions,            
                ["Encounter_E_06"] = Skulls_Ranks_01.Lieutenants,
                ["Encounter_E_07"] = Skulls_Ranks_01.Minions,
                ["Encounter_E_08"] = Skulls_Ranks_01.Minions,            
       },
}