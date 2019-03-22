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
        ["SkullMinions"] = {
                "Thug_Skull_01", "Thug_Skull_02", "Thug_Skull_03",
                "Thug_Skull_04", "Thug_Skull_05", "Thug_Skull_06",                
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

        },
}

local Vahzilok_Rumble_01  = {
        ["Minions"] = {
            "Slab_01", "Slab_02", "Slab_03",
            "Slab_04", "Cylok_01", "Cylok_02",
            "Cylok_03", "Cylok_04", "Cylok_Slab_01",
            "Cylok_Slab_02", "Cylok_Slab_03", "Cylok_Slab_04",
        },
        ["Lieutenants"] = {
            "Reaper_01", "Reaper_02", "Reaper_03",
            "Reaper_04", "Reaper_05", "Reaper_06",
            "Reaper_07", "Reaper_08", "Reaper_09",
            "Reaper_10", "Reaper_11", "Reaper_12",
            "Reaper_13", "Reaper_14", "Reaper_15",
            "Reaper_16",
        },
        ["Boss"] = {
            "Eidola_Male", "Eidola_Female",
        }
    }

--[[
        These are the spawndefs.
]]

Loiter_Hellions_L1_3_V0 = {
        ["Markers"] = {
            ["Encounter_S_31"] = Hellions_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Hellions_Ranks_01.Boss,            
            ["Encounter_E_06"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_05"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_07"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_08"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_01"] = Hellions_Ranks_01.Minions,            
       },
}

Loiter_Hellions_L1_3_V1 = Loiter_Hellions_L1_3_V0
Loiter_Hellions_L1_3_V2 = Loiter_Hellions_L1_3_V0
Loiter_Hellions_L1_3_V3 = Loiter_Hellions_L1_3_V0
Loiter_Hellions_L1_3_V4 = Loiter_Hellions_L1_3_V0

Loiter_Hellions_L4_7_V0 = {
        ["Markers"] = {
            ["Encounter_E_01"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_02"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_03"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_04"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_07"] = Hellions_Ranks_01.Minions,
            ["Encounter_E_05"] = Hellions_Ranks_01.Boss,
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
                ["Encounter_E_09"] = Hellions_Ranks_01.Minions,
                ["Encounter_E_03"] = Hellions_Ranks_01.Lieutenants,
                ["Encounter_E_01"] = Hellions_Ranks_01.Minions,
                ["Encounter_E_11"] = Hellions_Ranks_01.Minions,
                ["Encounter_E_05"] = Hellions_Ranks_01.Minions,
                ["Encounter_E_07"] = Hellions_Ranks_01.Lieutenants,
                ["Encounter_S_33"] = Hellions_Ranks_01.Minions,
                ["Encounter_S_31"] = Hellions_Ranks_01.Minions,
                ["Encounter_S_35"] = Hellions_Ranks_01.Minions,
                ["Encounter_E_02"] = Vahzilok_Rumble_01.Minions,
                ["Encounter_E_10"] = Vahzilok_Rumble_01.Minions,
                ["Encounter_E_04"] = Vahzilok_Rumble_01.Minions,
                ["Encounter_E_12"] = Vahzilok_Rumble_01.Lieutenants,
                ["Encounter_E_06"] = Vahzilok_Rumble_01.Lieutenants,
                ["Encounter_E_08"] = Vahzilok_Rumble_01.Minions,
                ["Encounter_S_32"] = Vahzilok_Rumble_01.Minions,
                ["Encounter_S_34"] = Vahzilok_Rumble_01.Minions,
                ["Encounter_S_30"] = Vahzilok_Rumble_01.Minions,
       },
}

Rumble_HellionsSkulls_L4_7_V1 = Rumble_HellionsSkulls_L4_7_V0
Rumble_HellionsSkulls_L4_7_V2 = Rumble_HellionsSkulls_L4_7_V0

Rumble_HellionsSkulls_L8_10_V0 = Rumble_HellionsSkulls_L4_7_V0
Rumble_HellionsSkulls_L8_10_V1 = Rumble_HellionsSkulls_L4_7_V0
Rumble_HellionsSkulls_L8_10_V2 = Rumble_HellionsSkulls_L4_7_V0

