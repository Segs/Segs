--This is a rank table
--There could be multiple tables to generate spawns from
local Hellion_Ranks_01  = {
        ["Underlings"] = {
          --NA
        },
        ["Minions"] = {
        "Thug_Hellion_01", "Thug_Hellion_02", "Thug_Hellion_03",
        "Thug_Hellion_04", "Thug_Hellion_05", "Thug_Hellion_06",
        },
        ["Lieutenants"] = {
          --NA
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
        "FemaleNPC_08", "FemaleNPC_13", "FemaleNPC_23",
        "FemaleNPC_12", "FemaleNPC_15", "MaleNPC_01",
        "MaleNPC_02", "MaleNPC_03", "MaleNPC_04",
        "MaleNPC_05", "MaleNPC_06",
        },
        ["Specials"] = {
        "Thug_Hellion_01", "Thug_Hellion_02", "Thug_Hellion_03",
        "Thug_Hellion_04", "Thug_Hellion_05", "Thug_Hellion_06",
        },
}

--[[
        These are the spawndefs.
]]


Mugging_Thugs_L1_3_V0 = {
        ["Markers"] = {
            ["Encounter_V_40"] = Hellion_Ranks_01.Victims,
            ["Encounter_S_30"] = Hellion_Ranks_01.Boss,
        },
}

Mugging_Thugs_L1_3_V1 = {
        ["Markers"] = {
            ["Encounter_V_40"] = Hellion_Ranks_01.Victims,
            ["Encounter_S_30"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_07"] = Hellion_Ranks_01.Minions,
        },
}

Mugging_Thugs_L4_7_V0 = {
        ["Markers"] = {
            ["Encounter_V_40"] = Hellion_Ranks_01.Victims,
            ["Encounter_S_30"] = Hellion_Ranks_01.Boss,
            ["Encounter_E_07"] = Hellion_Ranks_01.Minions,           
        },
}

Mugging_Thugs_L4_7_V1 = {
        ["Markers"] = {
            ["Encounter_V_40"] = Hellion_Ranks_01.Victims,
            ["Encounter_S_30"] = Hellion_Ranks_01.Boss,
            ["Encounter_E_01"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_03"] = Hellion_Ranks_01.Minions,
        },
}

Mugging_Thugs_L8_10_V0 = {
        ["Markers"] = {
            ["Encounter_V_40"] = Hellion_Ranks_01.Victims,
            ["Encounter_S_30"] = Hellion_Ranks_01.Boss,
            ["Encounter_E_06"] = Hellion_Ranks_01.Minions,
            ["Encounter_S_32"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_08"] = Hellion_Ranks_01.Minions,
        },
}

Mugging_Thugs_L8_10_V1 = {
        ["Markers"] = {
            ["Encounter_V_40"] = Hellion_Ranks_01.Victims,
            ["Encounter_S_30"] = Hellion_Ranks_01.Boss,
            ["Encounter_E_02"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_04"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_07"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_08"] = Hellion_Ranks_01.Minions,                                    
        },
}

ShadyDeal_Thugs_L1_3_V0 = {
        ["Markers"] = {
            ["Encounter_S_30"] = Hellion_Ranks_01.Boss,
            ["Encounter_V_40"] = Hellion_Ranks_01.Minions,
        },
}

ShadyDeal_Thugs_L1_3_V1 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Hellion_Ranks_01.Minions,
                ["Encounter_V_40"] = Hellion_Ranks_01.Minions,
        },
}

ShadyDeal_Thugs_L4_7_V0 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Hellion_Ranks_01.Minions,
                ["Encounter_V_40"] = Hellion_Ranks_01.Minions,
                ["Encounter_E_01"] = Hellion_Ranks_01.Minions,            
                ["Encounter_E_07"] = Hellion_Ranks_01.Minions,                
        },
}

ShadyDeal_Thugs_L4_7_V1 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Hellion_Ranks_01.Boss,
                ["Encounter_V_40"] = Hellion_Ranks_01.Boss,
                ["Encounter_E_01"] = Hellion_Ranks_01.Minions,            
                ["Encounter_E_02"] = Hellion_Ranks_01.Minions,
                ["Encounter_E_03"] = Hellion_Ranks_01.Minions,
                ["Encounter_E_06"] = Hellion_Ranks_01.Minions,
        },
}

ShadyDeal_Thugs_L8_10_V0 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Hellion_Ranks_01.Boss,
                ["Encounter_S_31"] = Hellion_Ranks_01.Boss,
                ["Encounter_V_40"] = Hellion_Ranks_01.Minions,
                ["Encounter_V_41"] = Hellion_Ranks_01.Minions,            
                ["Encounter_E_02"] = Hellion_Ranks_01.Minions,
        },
}

ShadyDeal_Thugs_L8_10_V1 = {
        ["Markers"] = {
                ["Encounter_S_30"] = Hellion_Ranks_01.Boss,
                ["Encounter_S_31"] = Hellion_Ranks_01.Boss,
                ["Encounter_V_40"] = Hellion_Ranks_01.Minions,
                ["Encounter_V_41"] = Hellion_Ranks_01.Minions,            
                ["Encounter_E_02"] = Hellion_Ranks_01.Minions,
                ["Encounter_E_06"] = Hellion_Ranks_01.Minions,
                ["Encounter_E_07"] = Hellion_Ranks_01.Minions,            
        },
}

Vandalism_Thugs_L1_3_V0 = {
        ["Markers"] = {
            ["Encounter_S_30"] = Hellion_Ranks_01.Minions,
        },
}

Vandalism_Thugs_L1_3_V1 = {
        ["Markers"] = {
            ["Encounter_S_30"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_01"] = Hellion_Ranks_01.Minions,             
        },
}

Vandalism_Thugs_L4_7_V0 = {
        ["Markers"] = {
            ["Encounter_S_30"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_04"] = Hellion_Ranks_01.Boss,            
        },
}

Vandalism_Thugs_L4_7_V1 = {
        ["Markers"] = {
            ["Encounter_S_30"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_02"] = Hellion_Ranks_01.Boss,
        },
}

Vandalism_Thugs_L8_10_V0 = {
        ["Markers"] = {
            ["Encounter_S_30"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_02"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_05"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_06"] = Hellion_Ranks_01.Minions,            
        },
}

Vandalism_Thugs_L8_10_V1 = {
        ["Markers"] = {
            ["Encounter_S_30"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_02"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_05"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_06"] = Hellion_Ranks_01.Minions,                                    
        },
}

BreakingIn_Thugs_L2_5_V0 = {
        ["Markers"] = {
            ["Encounter_S_30"] = Hellion_Ranks_01.Minions,
        },
}

BreakingIn_Thugs_L2_5_V1 = {
        ["Markers"] = {
            ["Encounter_S_31"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_05"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_06"] = Hellion_Ranks_01.Minions,
        },
}

BreakingIn_Thugs_L6_9_V0 = {
        ["Markers"] = {
            ["Encounter_S_30"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_01"] = Hellion_Ranks_01.Boss,            
            ["Encounter_E_02"] = Hellion_Ranks_01.Boss,            
        },
}

BreakingIn_Thugs_L6_9_V1 = {
        ["Markers"] = {
            ["Encounter_S_31"] = Hellion_Ranks_01.Minions,
            ["Encounter_S_30"] = Hellion_Ranks_01.Minions,            
            ["Encounter_E_01"] = Hellion_Ranks_01.Boss,
            ["Encounter_E_04"] = Hellion_Ranks_01.Boss,                        
        },
}

PurseSnatch_Thugs_L1_3_V0 = {
        ["Markers"] = {
            ["Encounter_V_40"] = Hellion_Ranks_01.Victims,
            ["Encounter_S_30"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_05"] = Hellion_Ranks_01.Minions,
        },
}

PurseSnatch_Thugs_L4_7_V0 = {
        ["Markers"] = {
            ["Encounter_V_40"] = Hellion_Ranks_01.Victims,
            ["Encounter_S_30"] = Hellion_Ranks_01.Boss,
            ["Encounter_E_03"] = Hellion_Ranks_01.Minions,
            ["Encounter_E_05"] = Hellion_Ranks_01.Minions,            
        },
}

PurseSnatch_Thugs_L8_10_V0 = {
        ["Markers"] = {
            ["Encounter_V_40"] = Hellion_Ranks_01.Victims,
            ["Encounter_V_41"] = Hellion_Ranks_01.Victims,            
            ["Encounter_S_30"] = Hellion_Ranks_01.Boss,
            ["Encounter_S_31"] = Hellion_Ranks_01.Minions,            
            ["Encounter_E_03"] = Hellion_Ranks_01.Minions,            
            ["Encounter_E_05"] = Hellion_Ranks_01.Boss,
        },
}