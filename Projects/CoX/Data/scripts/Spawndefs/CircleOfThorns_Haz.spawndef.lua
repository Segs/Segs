--This is a rank table
--There could be multiple tables to generate spawns from
local Circle_Ranks_01  = {
        ["Underlings"] = {
          --NA
        },
        ["Minions"] = {
            "CoT_01","CoT_02","CoT_03",
            "CoT_11","CoT_21","CoT_22",
            "CoT_23","CoT_24",
            "CoT_31","CoT_32",
            "CoT_33","CoT_34",
            "CoT_41","CoT_42",
            "CoT_43","CoT_44",
        },
        ["Lieutenants"] = {
            "Daemon", "Knight", "Spectre",
            "Daemon_Lord", "Behemoth_Minion",
        },
        ["Sniper"] = {
          --NA
        },
        ["Boss"] = {
            "Behemoth_Lord", "Behemoth_Envoy"
        },
        ["Elite Boss"] = {
            "CoTUniqueThreatLevel1", "CoTUniqueThreatLevel2"
        },
        ["Victims"] = {

        },
        ["Specials"] = {

        },
}

--[[
        These are the spawndefs.
]]

Sacrifice_CircleOfThornsFollowe_L5_9_V0 = {
        ["Around"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V1 = {
        ["Around"] = {
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_E_01"] = Circle_Ranks_01.Lieutenants,                
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V2 = {
        ["Around"] = {
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Minions,                
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V3 = {
        ["Around"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V4 = {
        ["Around"] = {
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_E_01"] = Circle_Ranks_01.Lieutenants,                
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V5 = {
        ["Around"] = {
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Minions,                
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V6 = {
        ["Around"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V7 = {
        ["Around"] = {
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_E_01"] = Circle_Ranks_01.Lieutenants,                
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V8 = {
        ["Around"] = {
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Minions,                
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V9 = {
        ["Around"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_04"] = Circle_Ranks_01.Minions,
                ["Encounter_E_08"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Boss,
                ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,                  
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V10 = {
        ["Around"] = {
            ["Encounter_E_01"] = Circle_Ranks_01.Minions,
            ["Encounter_E_02"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_04"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_06"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_08"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V11 = {
        ["Around"] = {
            ["Encounter_E_01"] = Circle_Ranks_01.Minions,
            ["Encounter_E_02"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_04"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_06"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_08"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,               
        },
}
Sacrifice_CircleOfThornsFollowe_L5_9_V12 = {
        ["Around"] = {
            ["Encounter_E_01"] = Circle_Ranks_01.Minions,
            ["Encounter_E_02"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_04"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_06"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_08"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,               
        },
}
Sacrifice_CircleOfThornsFollowe_L5_9_V13 = {
        ["Around"] = {
            ["Encounter_E_01"] = Circle_Ranks_01.Minions,
            ["Encounter_E_02"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_04"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_06"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_08"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,               
        },
}
Sacrifice_CircleOfThornsFollowe_L5_9_V14 = {
        ["Around"] = {
            ["Encounter_E_01"] = Circle_Ranks_01.Minions,
            ["Encounter_E_02"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_04"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_06"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_08"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,              
        },
}
Sacrifice_CircleOfThornsFollowe_L5_9_V15 = {
        ["Around"] = {
            ["Encounter_E_01"] = Circle_Ranks_01.Minions,
            ["Encounter_E_02"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_04"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_06"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_08"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,               
        },
}


-- =======================================================

Sacrifice_CircleOfThornsFollowe_L2_5_V0 = {
        ["Around"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V1 = {
        ["Around"] = {
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_E_01"] = Circle_Ranks_01.Lieutenants,                
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V2 = {
        ["Around"] = {
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Minions,                
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V3 = {
        ["Around"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V4 = {
        ["Around"] = {
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_E_01"] = Circle_Ranks_01.Lieutenants,                
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V5 = {
        ["Around"] = {
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Minions,                
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V6 = {
        ["Around"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V7 = {
        ["Around"] = {
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_E_01"] = Circle_Ranks_01.Lieutenants,                
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V8 = {
        ["Around"] = {
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Minions,                
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V9 = {
        ["Around"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_04"] = Circle_Ranks_01.Minions,
                ["Encounter_E_08"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Boss,
                ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,                  
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V10 = {
        ["Around"] = {
            ["Encounter_E_01"] = Circle_Ranks_01.Minions,
            ["Encounter_E_02"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_04"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_06"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_08"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V11 = {
        ["Around"] = {
            ["Encounter_E_01"] = Circle_Ranks_01.Minions,
            ["Encounter_E_02"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_04"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_06"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_08"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,               
        },
}
Sacrifice_CircleOfThornsFollowe_L2_5_V12 = {
        ["Around"] = {
            ["Encounter_E_01"] = Circle_Ranks_01.Minions,
            ["Encounter_E_02"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_04"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_06"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_08"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,               
        },
}
Sacrifice_CircleOfThornsFollowe_L2_5_V13 = {
        ["Around"] = {
            ["Encounter_E_01"] = Circle_Ranks_01.Minions,
            ["Encounter_E_02"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_04"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_06"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_08"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,               
        },
}
Sacrifice_CircleOfThornsFollowe_L2_5_V14 = {
        ["Around"] = {
            ["Encounter_E_01"] = Circle_Ranks_01.Minions,
            ["Encounter_E_02"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_04"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_06"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_08"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,              
        },
}
Sacrifice_CircleOfThornsFollowe_L2_5_V15 = {
        ["Around"] = {
            ["Encounter_E_01"] = Circle_Ranks_01.Minions,
            ["Encounter_E_02"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_04"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_06"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_08"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,               
        },
}