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
        },
        ["Sniper"] = {
          --NA
        },
        ["Boss"] = {
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

Baphomet_D10_V0 = {
        ["Markers"] = {
                ["Encounter_E_01"] = "Behemoth_Lord",
        },
}


Sacrifice_CircleOfThornsFollowe_L5_9_V0 = {
        ["Markers"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V1 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_E_01"] = Circle_Ranks_01.Lieutenants,                
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V2 = {
        ["Markers"] = {
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Minions,                
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V3 = {
        ["Markers"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V4 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_E_01"] = Circle_Ranks_01.Lieutenants,                
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V5 = {
        ["Markers"] = {
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Minions,                
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V6 = {
        ["Markers"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V7 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_E_01"] = Circle_Ranks_01.Lieutenants,                
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V8 = {
        ["Markers"] = {
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Minions,                
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V9 = {
        ["Markers"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_04"] = Circle_Ranks_01.Minions,
                ["Encounter_E_08"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,                  
        },
}

Sacrifice_CircleOfThornsFollowe_L5_9_V10 = {
        ["Markers"] = {
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
        ["Markers"] = {
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
        ["Markers"] = {
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
        ["Markers"] = {
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
        ["Markers"] = {
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
        ["Markers"] = {
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

Sacrifice_CircleOfThornsFollowe_L2_5_V0 = {
        ["Markers"] = {
                ["Encounter_E_06"] = Circle_Ranks_01.Minions,
                ["Encounter_E_08"] = Circle_Ranks_01.Lieutenants,
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V1 = {
        ["Markers"] = {
                ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_06"] = Circle_Ranks_01.Minions,
                ["Encounter_E_08"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,                
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V2 = {
        ["Markers"] = {
                ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_06"] = Circle_Ranks_01.Minions,
                ["Encounter_E_08"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_04"] = Circle_Ranks_01.Minions,
                ["Encounter_E_03"] = Circle_Ranks_01.Minions,
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,                                                                
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V3 = {
        ["Markers"] = {
                ["Encounter_S_31"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_06"] = Circle_Ranks_01.Minions,
                ["Encounter_E_08"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_04"] = Circle_Ranks_01.Minions,
                ["Encounter_E_03"] = Circle_Ranks_01.Minions,
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V4 = {
        ["Markers"] = {
                ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_04"] = Circle_Ranks_01.Minions,
                ["Encounter_E_03"] = Circle_Ranks_01.Minions,
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,                
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V5 = {
        ["Markers"] = {
                ["Encounter_E_03"] = Circle_Ranks_01.Minions,
                ["Encounter_E_04"] = Circle_Ranks_01.Minions,
                ["Encounter_E_06"] = Circle_Ranks_01.Minions,
                ["Encounter_E_08"] = Circle_Ranks_01.Minions,
                ["Encounter_S_31"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Minions,                
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V6 = {
        ["Markers"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V7 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_E_01"] = Circle_Ranks_01.Lieutenants,                
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V8 = {
        ["Markers"] = {
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Minions,
                ["Encounter_S_32"] = Circle_Ranks_01.Minions,                
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V9 = {
        ["Markers"] = {
                ["Encounter_E_05"] = Circle_Ranks_01.Minions,
                ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_E_01"] = Circle_Ranks_01.Minions,
                ["Encounter_E_02"] = Circle_Ranks_01.Minions,
                ["Encounter_E_04"] = Circle_Ranks_01.Minions,
                ["Encounter_E_08"] = Circle_Ranks_01.Minions,
                ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
                ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,                  
        },
}

Sacrifice_CircleOfThornsFollowe_L2_5_V10 = {
        ["Markers"] = {
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
        ["Markers"] = {
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
        ["Markers"] = {
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
        ["Markers"] = {
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
        ["Markers"] = {
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
        ["Markers"] = {
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