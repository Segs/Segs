--This is a rank table
--There could be multiple tables to generate spawns from
local Vahzilok_Ranks_01  = {
        ["Underlings"] = {
          --NA
        },
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
        ["Sniper"] = {
          --NA
        },
        ["Boss"] = {
                "Eidola_Male", "Eidola_Female"
        },
        ["Elite Boss"] = {
          --NA
        },
        ["Victims"] = {

        },
        ["Specials"] = {

        },
}

--[[
        These are the spawndefs.
]]


Loiter_Vahzilok_L2_5_V0 = {
        ["Markers"] = {
                ["Encounter_E_01"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_05"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_07"] = Vahzilok_Ranks_01.Lieutenants,
                ["Encounter_E_04"] = Vahzilok_Ranks_01.Lieutenants,
        },
}

Loiter_Vahzilok_L2_5_V1 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Vahzilok_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_07"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_01"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_S_31"] = Vahzilok_Ranks_01.Minions,
        },
}

Loiter_Vahzilok_L2_5_V2 = {
        ["Markers"] = {
                ["Encounter_E_01"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_02"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_05"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_07"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_S_30"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_S_32"] = Vahzilok_Ranks_01.Minions,                
        },
}

Loiter_Vahzilok_L5_9_V0 = {
        ["Markers"] = {
                ["Encounter_E_05"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_07"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_01"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_S_31"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_S_32"] = Vahzilok_Ranks_01.Lieutenants,
        },
}

Loiter_Vahzilok_L5_9_V1 = {
        ["Markers"] = {
                ["Encounter_E_01"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_02"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_06"] = Vahzilok_Ranks_01.Minions,                
                ["Encounter_S_32"] = Vahzilok_Ranks_01.Lieutenants,                
        },
}

Loiter_Vahzilok_L5_9_V2 = {
        ["Markers"] = {
                ["Encounter_E_01"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_03"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_06"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_E_07"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_S_30"] = Vahzilok_Ranks_01.Minions,
                ["Encounter_S_32"] = Vahzilok_Ranks_01.Minions,                
        },
}