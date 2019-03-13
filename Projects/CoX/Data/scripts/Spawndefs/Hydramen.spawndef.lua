--This is a rank table
--There could be multiple tables to generate spawns from
local Hydra_Ranks_01  = {
        ["Underlings"] = {
            "Hydra_Melee_Small", "Hydra_Range_Small"
        },
        ["Minions"] = {
            "Hydra_Slug_Minion",
        },
        ["Lieutenants"] = {
            "Hydra_Slug_Lieutenant",  "Hydra_Melee_Large", "Hydra_Range_Large"
        },
        ["Sniper"] = {
            "Hydra_Pod"
        },
        ["Boss"] = {
            "Hydra_Slug_Boss"
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


Loiter_HydraMan_L7_9_V0 = {
        ["Around"] = {
                ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_07"] = Hydra_Ranks_01.Lieutenants,
        },
}

Loiter_HydraMan_L7_9_V1 = {
        ["Around"] = {
                ["Encounter_S_30"] = Hydra_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Hydra_Ranks_01.Underlings,
                ["Encounter_E_07"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_01"] = Hydra_Ranks_01.Lieutenants,                
        },
}

Loiter_HydraMan_L7_9_V2 = {
        ["Around"] = {
                ["Encounter_E_01"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_02"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_03"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_06"] = Hydra_Ranks_01.Underlings,
                ["Encounter_E_08"] = Hydra_Ranks_01.Underlings,
                ["Encounter_S_32"] = Hydra_Ranks_01.Minions,                
        },
}

Loiter_HydraMan_L7_9_V3 = {
        ["Around"] = {
                ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_07"] = Hydra_Ranks_01.Lieutenants,
        },
}

Loiter_HydraMan_L7_9_V4 = {
        ["Around"] = {
                ["Encounter_S_30"] = Hydra_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_07"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_01"] = Hydra_Ranks_01.Lieutenants,                
        },
}

Loiter_HydraMan_L7_9_V5 = {
        ["Around"] = {
                ["Encounter_E_01"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_02"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_07"] = Hydra_Ranks_01.Minions,
                ["Encounter_S_30"] = Hydra_Ranks_01.Minions,
                ["Encounter_S_32"] = Hydra_Ranks_01.Minions,                
        },
}

Loiter_HydraMan_L7_9_V6 = {
        ["Around"] = {
                ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_07"] = Hydra_Ranks_01.Lieutenants,
        },
}

Loiter_HydraMan_L7_9_V7 = {
        ["Around"] = {
                ["Encounter_S_30"] = Hydra_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_07"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_01"] = Hydra_Ranks_01.Lieutenants,                
        },
}

Loiter_HydraMan_L7_9_V8 = {
        ["Around"] = {
                ["Encounter_E_01"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_02"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_07"] = Hydra_Ranks_01.Minions,
                ["Encounter_S_30"] = Hydra_Ranks_01.Minions,
                ["Encounter_S_32"] = Hydra_Ranks_01.Minions,                
        },
}

Loiter_HydraMan_L7_9_V9 = {
        ["Around"] = {
                ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_07"] = Hydra_Ranks_01.Lieutenants,
                ["Encounter_E_01"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_02"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_04"] = Hydra_Ranks_01.Minions,
                ["Encounter_E_08"] = Hydra_Ranks_01.Minions,
                ["Encounter_S_30"] = Hydra_Ranks_01.Boss,
                ["Encounter_S_32"] = Hydra_Ranks_01.Lieutenants,                  
        },
}

Loiter_HydraMan_L7_9_V10 = {
        ["Around"] = {
            ["Encounter_E_01"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_02"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_03"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_04"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_06"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_07"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_08"] = Hydra_Ranks_01.Minions,
            ["Encounter_S_30"] = Hydra_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Hydra_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Hydra_Ranks_01.Lieutenants,
        },
}

Loiter_HydraMan_L7_9_V11 = {
        ["Around"] = {
            ["Encounter_E_01"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_02"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_03"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_04"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_06"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_07"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_08"] = Hydra_Ranks_01.Minions,
            ["Encounter_S_30"] = Hydra_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Hydra_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Hydra_Ranks_01.Lieutenants,               
        },
}
Loiter_HydraMan_L7_9_V12 = {
        ["Around"] = {
            ["Encounter_E_01"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_02"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_03"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_04"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_06"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_07"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_08"] = Hydra_Ranks_01.Minions,
            ["Encounter_S_30"] = Hydra_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Hydra_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Hydra_Ranks_01.Lieutenants,               
        },
}
Loiter_HydraMan_L7_9_V13 = {
        ["Around"] = {
            ["Encounter_E_01"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_02"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_03"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_04"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_06"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_07"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_08"] = Hydra_Ranks_01.Minions,
            ["Encounter_S_30"] = Hydra_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Hydra_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Hydra_Ranks_01.Lieutenants,               
        },
}
Loiter_HydraMan_L7_9_V14 = {
        ["Around"] = {
            ["Encounter_E_01"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_02"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_03"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_04"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_06"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_07"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_08"] = Hydra_Ranks_01.Minions,
            ["Encounter_S_30"] = Hydra_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Hydra_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Hydra_Ranks_01.Lieutenants,              
        },
}
Loiter_HydraMan_L7_9_V15 = {
        ["Around"] = {
            ["Encounter_E_01"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_02"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_03"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_04"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_05"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_06"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_07"] = Hydra_Ranks_01.Minions,
            ["Encounter_E_08"] = Hydra_Ranks_01.Minions,
            ["Encounter_S_30"] = Hydra_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Hydra_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Hydra_Ranks_01.Lieutenants,               
        },
}