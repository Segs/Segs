--This is a rank table
--There could be multiple tables to generate spawns from
local Outcasts_Ranks_01  = {
    ["Underlings"] = {
      --NA
    },
    ["Minions"] = {
        "Thug_Outcast_01", "Thug_Outcast_02",
        "Thug_Outcast_03", "Thug_Outcast_04",
        "Thug_Outcast_05", "Thug_Outcast_06",
        "Thug_Outcast_07", "Thug_Outcast_08",
    },
    ["Lieutenants"] = {
        "Thug_Outcast_Boss_01", "Thug_Outcast_Boss_02",
        "Thug_Outcast_Boss_03", "Thug_Outcast_Boss_04",
        "Thug_Outcast_Boss_05", "Thug_Outcast_Boss_06",
        "Thug_Outcast_Boss_07", "Thug_Outcast_Boss_08",
    },
    ["Sniper"] = {
      --NA
    },
    ["Boss"] = {

    },
    ["Elite Boss"] = {
      --NA
    },
    ["Victims"] = {
        "FemaleNPC_51", "FemaleNPC_56", "FemaleNPC_52",
        "FemaleNPC_53", "FemaleNPC_54", "FemaleNPC_55",
        "MaleNPC_50", "MaleNPC_51", "MaleNPC_52",
        "MaleNPC_53", "MaleNPC_54", "MaleNPC_55",
        "MaleNPC_56", "MaleNPC_57", "MaleNPC_58",
        "MaleNPC_59",
    },
    ["FemaleVictims"] = {
        "FemaleNPC_51", "FemaleNPC_56", "FemaleNPC_52",
        "FemaleNPC_54", "FemaleNPC_58", "FemaleNPC_57",         
    },
    ["Specials"] = {

    },
}

local Trolls_Rumble_01  = {
    ["Minions"] = {
        "Thug_Troll_01", "Thug_Troll_02",
        "Thug_Troll_03", "Thug_Troll_04",
        "Thug_Troll_05", "Thug_Troll_06",
    },
    ["Lieutenants"] = {
        "Thug_Troll_Boss_01",
        "Thug_Troll_Boss_02",
        "Thug_Troll_Boss_03",
    },
    ["Boss"] = {

    }
}

local Family_Rumble_01  = {
    ["Minions"] = {
        "Thug_Family_01", "Thug_Family_02",
        "Thug_Family_03", "Thug_Family_04",
        "Thug_Family_05", "Thug_Family_06",
    },
    ["Lieutenants"] = {
        "Thug_Family_Boss_01",
        "Thug_Family_Boss_02",
    },
    ["Boss"] = {

    }
}

--== MUGGINGS ==--

Mugging_Outcasts_D1_V0 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Outcasts_Ranks_01.Victims,
            ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_07"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_08"] = Outcasts_Ranks_01.Minions,
    },
}

Mugging_Outcasts_D1_V1 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Outcasts_Ranks_01.Victims,
            ["Encounter_S_30"] = Outcasts_Ranks_01.Lieutenants,
            ["Encounter_E_07"] = Outcasts_Ranks_01.Minions,
    },
}

Mugging_Outcasts_D1_V2 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Outcasts_Ranks_01.Victims,
            ["Encounter_S_30"] = Outcasts_Ranks_01.Lieutenants,
            ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_07"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_01"] = Outcasts_Ranks_01.Minions,
    },
}

Mugging_Outcasts_D1_V3 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Outcasts_Ranks_01.Victims,
            ["Encounter_V_40"] = Outcasts_Ranks_01.Victims,
            ["Encounter_S_30"] = Outcasts_Ranks_01.Lieutenants,
            ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_07"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_01"] = Outcasts_Ranks_01.Lieutenants,
            ["Encounter_E_03"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_04"] = Outcasts_Ranks_01.Minions,
    },
}

Mugging_Outcasts_D5_V0 = Mugging_Outcasts_D1_V0
Mugging_Outcasts_D5_V1 = Mugging_Outcasts_D1_V1
Mugging_Outcasts_D5_V2 = Mugging_Outcasts_D1_V2
Mugging_Outcasts_D5_V3 = Mugging_Outcasts_D1_V3

Mugging_Outcasts_D9_V0 = Mugging_Outcasts_D1_V0
Mugging_Outcasts_D9_V1 = Mugging_Outcasts_D1_V1
Mugging_Outcasts_D9_V2 = Mugging_Outcasts_D1_V2
Mugging_Outcasts_D9_V3 = Mugging_Outcasts_D1_V3

--== SHADY DEALS ==--

ShadyDeal_Outcasts_D1_V0 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
            ["Encounter_S_32"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_01"] = Outcasts_Ranks_01.Minions,
    },
}

ShadyDeal_Outcasts_D1_V1 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Outcasts_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Outcasts_Ranks_01.Lieutenants,
            ["Encounter_E_01"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_04"] = Outcasts_Ranks_01.Minions,
    },
}

ShadyDeal_Outcasts_D1_V2 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_S_32"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_01"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_04"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_08"] = Outcasts_Ranks_01.Minions,
    },
}

ShadyDeal_Outcasts_D5_V0 = ShadyDeal_Outcasts_D1_V0
ShadyDeal_Outcasts_D5_V1 = ShadyDeal_Outcasts_D1_V1
ShadyDeal_Outcasts_D5_V2 = ShadyDeal_Outcasts_D1_V2

ShadyDeal_Outcasts_D9_V0 = ShadyDeal_Outcasts_D1_V0
ShadyDeal_Outcasts_D9_V1 = ShadyDeal_Outcasts_D1_V1
ShadyDeal_Outcasts_D9_V2 = ShadyDeal_Outcasts_D1_V2

--== FIGHT CLUB ==--

FightClub_Outcasts_D3_V0 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
            ["Encounter_S_32"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_01"] = Outcasts_Ranks_01.Minions,
    },
}

FightClub_Outcasts_D3_V1 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
            ["Encounter_S_32"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_01"] = Outcasts_Ranks_01.Lieutenants,
            ["Encounter_E_04"] = Outcasts_Ranks_01.Lieutenants,
    },
}

FightClub_Outcasts_D3_V2 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_32"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_01"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_04"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_08"] = Outcasts_Ranks_01.Minions,
    },
}

FightClub_Outcasts_D8_V0 = FightClub_Outcasts_D3_V0
FightClub_Outcasts_D8_V1 = FightClub_Outcasts_D3_V1
FightClub_Outcasts_D8_V2 = FightClub_Outcasts_D3_V2

--== LOITERING ==--

Loiter_Outcasts_D3_V0 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
            ["Encounter_S_32"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_01"] = Outcasts_Ranks_01.Minions,
    },
}

Loiter_Outcasts_D3_V1 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
            ["Encounter_S_32"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_01"] = Outcasts_Ranks_01.Lieutenants,
            ["Encounter_E_04"] = Outcasts_Ranks_01.Lieutenants,
    },
}

Loiter_Outcasts_D3_V2 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_32"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_01"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_04"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_08"] = Outcasts_Ranks_01.Minions,
    },
}

Loiter_Outcasts_D8_V0 = Loiter_Outcasts_D3_V0
Loiter_Outcasts_D8_V1 = Loiter_Outcasts_D3_V1
Loiter_Outcasts_D8_V2 = Loiter_Outcasts_D3_V2

--== PURSE SNATCH ==--

PurseSnatch_Outcasts_D1_V0 = {
    ["Markers"] = {
        ["Encounter_V_40"] = Outcasts_Ranks_01.FemaleVictims,
        ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
    },
}

PurseSnatch_Outcasts_D1_V1 = {
    ["Markers"] = {
        ["Encounter_V_40"] = Outcasts_Ranks_01.FemaleVictims,
        ["Encounter_S_30"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_03"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,            
    },
}

PurseSnatch_Outcasts_D1_V2 = {
    ["Markers"] = {
        ["Encounter_V_40"] = Outcasts_Ranks_01.FemaleVictims,
        ["Encounter_V_41"] = Outcasts_Ranks_01.Victims,            
        ["Encounter_S_30"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_S_31"] = Outcasts_Ranks_01.Minions,            
        ["Encounter_E_03"] = Outcasts_Ranks_01.Minions,            
        ["Encounter_E_05"] = Outcasts_Ranks_01.Lieutenants,
    },
}

PurseSnatch_Outcasts_D5_V0 = PurseSnatch_Outcasts_D1_V0
PurseSnatch_Outcasts_D5_V1 = PurseSnatch_Outcasts_D1_V1
PurseSnatch_Outcasts_D5_V2 = PurseSnatch_Outcasts_D1_V2

PurseSnatch_Outcasts_D9_V0 = PurseSnatch_Outcasts_D1_V0
PurseSnatch_Outcasts_D9_V1 = PurseSnatch_Outcasts_D1_V1
PurseSnatch_Outcasts_D9_V2 = PurseSnatch_Outcasts_D1_V2

--== VANDALISM ==--

Vandalism_Outcasts_D3_V0 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_02"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
    },
}

Vandalism_Outcasts_D3_V1 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_02"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_07"] = Outcasts_Ranks_01.Lieutenants,                                    
        ["Encounter_E_01"] = Outcasts_Ranks_01.Minions,
    },
}

Vandalism_Outcasts_D3_V2 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_31"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_06"] = Outcasts_Ranks_01.Minions,                                    
        ["Encounter_E_03"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_02"] = Outcasts_Ranks_01.Minions,
    },
}

Vandalism_Outcasts_D8_V0 = Vandalism_Outcasts_D3_V0
Vandalism_Outcasts_D8_V1 = Vandalism_Outcasts_D3_V1
Vandalism_Outcasts_D8_V2 = Vandalism_Outcasts_D3_V2

--== BREAK-INS ==--

BreakingIn_Outcasts_D3_V0 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_01"] = Outcasts_Ranks_01.Lieutenants,            
        ["Encounter_E_06"] = Outcasts_Ranks_01.Lieutenants,            
    },
}

BreakingIn_Outcasts_D3_V1 = {
    ["Markers"] = {
        ["Encounter_S_31"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,            
        ["Encounter_E_01"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_04"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_06"] = Outcasts_Ranks_01.Minions,
    },
}

BreakingIn_Outcasts_D3_V2 = {
    ["Markers"] = {
        ["Encounter_S_31"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,            
        ["Encounter_E_01"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_02"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_04"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_06"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_08"] = Outcasts_Ranks_01.Minions,                        
    },
}

BreakingIn_Outcasts_D8_V0 = BreakingIn_Outcasts_D3_V0
BreakingIn_Outcasts_D8_V1 = BreakingIn_Outcasts_D3_V1
BreakingIn_Outcasts_D8_V2 = BreakingIn_Outcasts_D3_V2

--==RUMBLE: Outcasts vs. Trolls ==--

Rumble_OutcastsTrolls_D1_V0 = {
    ["Markers"] = {
        ["Encounter_E_09"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_03"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_01"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_33"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_31"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_02"] = Trolls_Rumble_01.Minions,
        ["Encounter_E_10"] = Trolls_Rumble_01.Minions,
        ["Encounter_E_04"] = Trolls_Rumble_01.Minions,
        ["Encounter_E_06"] = Trolls_Rumble_01.Lieutenants,
        ["Encounter_S_32"] = Trolls_Rumble_01.Minions,
        ["Encounter_S_30"] = Trolls_Rumble_01.Minions,           
   },
}

Rumble_OutcastsTrolls_D1_V1 = {
    ["Markers"] = {
        ["Encounter_E_09"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_03"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_01"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_33"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_31"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_35"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_10"] = Trolls_Rumble_01.Minions,
        ["Encounter_E_12"] = Trolls_Rumble_01.Lieutenants,
        ["Encounter_E_08"] = Trolls_Rumble_01.Lieutenants,
        ["Encounter_S_32"] = Trolls_Rumble_01.Minions,
        ["Encounter_S_34"] = Trolls_Rumble_01.Lieutenants,
        ["Encounter_S_30"] = Trolls_Rumble_01.Minions,           
   },
}

Rumble_OutcastsTrolls_D1_V2 = {
    ["Markers"] = {
            ["Encounter_E_09"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_03"] = Outcasts_Ranks_01.Lieutenants,
            ["Encounter_E_01"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_11"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_07"] = Outcasts_Ranks_01.Lieutenants,
            ["Encounter_S_33"] = Outcasts_Ranks_01.Minions,
            ["Encounter_S_31"] = Outcasts_Ranks_01.Minions,
            ["Encounter_S_35"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_02"] = Family_Rumble_01.Minions,
            ["Encounter_E_10"] = Family_Rumble_01.Minions,
            ["Encounter_E_04"] = Family_Rumble_01.Minions,
            ["Encounter_E_12"] = Family_Rumble_01.Lieutenants,
            ["Encounter_E_06"] = Family_Rumble_01.Lieutenants,
            ["Encounter_E_08"] = Family_Rumble_01.Minions,
            ["Encounter_S_32"] = Family_Rumble_01.Minions,
            ["Encounter_S_34"] = Family_Rumble_01.Minions,
            ["Encounter_S_30"] = Family_Rumble_01.Minions,
   },
}

Rumble_OutcastsTrolls_D5_V0 = Rumble_OutcastsTrolls_D1_V0
Rumble_OutcastsTrolls_D5_V1 = Rumble_OutcastsTrolls_D1_V1
Rumble_OutcastsTrolls_D5_V2 = Rumble_OutcastsTrolls_D1_V2

--==RUMBLE: Outcasts vs. Family ==--

Negotiation_OutcastsFamily_D3_V0 = {
    ["Markers"] = {
        ["Encounter_E_03"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_33"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_31"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_10"] = Family_Rumble_01.Minions,
        ["Encounter_E_06"] = Family_Rumble_01.Lieutenants,
        ["Encounter_S_32"] = Family_Rumble_01.Minions,
        ["Encounter_S_30"] = Family_Rumble_01.Minions,           
   },
}

Negotiation_OutcastsFamily_D3_V1 = {
    ["Markers"] = {
        ["Encounter_E_01"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_33"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_31"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_35"] = Outcasts_Ranks_01.Lieutenants,
        ["Encounter_E_12"] = Family_Rumble_01.Lieutenants,
        ["Encounter_E_08"] = Family_Rumble_01.Lieutenants,
        ["Encounter_S_34"] = Family_Rumble_01.Lieutenants,
        ["Encounter_S_30"] = Family_Rumble_01.Minions,           
   },
}

Negotiation_OutcastsFamily_D3_V2 = {
    ["Markers"] = {
            ["Encounter_E_09"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_03"] = Outcasts_Ranks_01.Lieutenants,
            ["Encounter_E_01"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_07"] = Outcasts_Ranks_01.Lieutenants,
            ["Encounter_S_33"] = Outcasts_Ranks_01.Minions,
            ["Encounter_S_31"] = Outcasts_Ranks_01.Minions,
            ["Encounter_S_35"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_02"] = Family_Rumble_01.Minions,
            ["Encounter_E_10"] = Family_Rumble_01.Minions,
            ["Encounter_E_04"] = Family_Rumble_01.Lieutenants,
            ["Encounter_E_08"] = Family_Rumble_01.Minions,
            ["Encounter_S_32"] = Family_Rumble_01.Minions,
            ["Encounter_S_34"] = Family_Rumble_01.Lieutenants,
            ["Encounter_S_30"] = Family_Rumble_01.Minions,
   },
}

Negotiation_OutcastsFamily_D8_V0 = Negotiation_OutcastsFamily_D3_V0
Negotiation_OutcastsFamily_D8_V1 = Negotiation_OutcastsFamily_D3_V1
Negotiation_OutcastsFamily_D8_V2 = Negotiation_OutcastsFamily_D3_V2