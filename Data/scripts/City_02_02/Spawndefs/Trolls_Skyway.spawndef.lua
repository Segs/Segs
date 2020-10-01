--This is a rank table
--There could be multiple tables to generate spawns from
local Trolls_Ranks_01  = {
    ["Underlings"] = {
      --NA
    },
    ["Minions"] = {
        "Thug_Troll_01","Thug_Troll_02","Thug_Troll_03",
        "Thug_Troll_04","Thug_Troll_05","Thug_Troll_06",
    },
    ["Boss"] = {

    },
    ["Sniper"] = {
      --NA
    },
    ["Boss"] = {
        "Thug_Troll_Boss_01", "Thug_Troll_Boss_02", "Thug_Troll_Boss_03"
    },
    ["Elite Boss"] = {
    },
    ["Victims"] = {
        "FemaleNPC_51", "FemaleNPC_56", "FemaleNPC_52",
        "FemaleNPC_53", "FemaleNPC_54", "FemaleNPC_55",
        "MaleNPC_50", "MaleNPC_51", "MaleNPC_52",
        "MaleNPC_53", "MaleNPC_54", "MaleNPC_55",
        "MaleNPC_56", "MaleNPC_57", "MaleNPC_58",
        "MaleNPC_59",
    },
    ["Specials"] = {

    },
}

-- ShadyDeals  --
ShadyDeal_Trolls_L11_13_V0 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Trolls_Ranks_01.Minions,
            ["Encounter_S_32"] = Trolls_Ranks_01.Minions,
            ["Encounter_E_01"] = Trolls_Ranks_01.Minions,
    },
}

ShadyDeal_Trolls_L11_13_V1 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Trolls_Ranks_01.Boss,
            ["Encounter_S_32"] = Trolls_Ranks_01.Boss,
            ["Encounter_E_01"] = Trolls_Ranks_01.Minions,
            ["Encounter_E_04"] = Trolls_Ranks_01.Minions,
    },
}

ShadyDeal_Trolls_L11_13_V2 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Trolls_Ranks_01.Boss,
        ["Encounter_S_32"] = Trolls_Ranks_01.Boss,
        ["Encounter_E_01"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_04"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_05"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_08"] = Trolls_Ranks_01.Minions,
    },
}

ShadyDeal_Trolls_L11_13_V3 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Trolls_Ranks_01.Boss,
        ["Encounter_S_32"] = Trolls_Ranks_01.Boss,
        ["Encounter_E_02"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_03"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_05"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_06"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_08"] = Trolls_Ranks_01.Minions,
    },
}

ShadyDeal_Trolls_L14_17_V0 = ShadyDeal_Trolls_L11_13_V0
ShadyDeal_Trolls_L14_17_V1 = ShadyDeal_Trolls_L11_13_V1
ShadyDeal_Trolls_L14_17_V2 = ShadyDeal_Trolls_L11_13_V2
ShadyDeal_Trolls_L14_17_V3 = ShadyDeal_Trolls_L11_13_V3

ShadyDeal_Trolls_L18_20_V0 = ShadyDeal_Trolls_L11_13_V0
ShadyDeal_Trolls_L18_20_V1 = ShadyDeal_Trolls_L11_13_V1
ShadyDeal_Trolls_L18_20_V2 = ShadyDeal_Trolls_L11_13_V2
ShadyDeal_Trolls_L18_20_V3 = ShadyDeal_Trolls_L11_13_V3

-- Fight Club --

FightClub_Trolls_L12_15_V0 = {
    ["Markers"] = {
        ["Encounter_S_32"] = Trolls_Ranks_01.Boss,
        ["Encounter_S_30"] = Trolls_Ranks_01.Boss,
        ["Encounter_E_03"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_07"] = Trolls_Ranks_01.Minions,            
    },
}

FightClub_Trolls_L12_15_V1 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Trolls_Ranks_01.Minions,
            ["Encounter_S_32"] = Trolls_Ranks_01.Minions,
            ["Encounter_E_01"] = Trolls_Ranks_01.Boss,
            ["Encounter_E_04"] = Trolls_Ranks_01.Boss,
    },
}

FightClub_Trolls_L12_15_V2 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Trolls_Ranks_01.Minions,
        ["Encounter_S_32"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_05"] = Trolls_Ranks_01.Boss,
        ["Encounter_E_06"] = Trolls_Ranks_01.Boss,
        ["Encounter_E_07"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_08"] = Trolls_Ranks_01.Minions,
    },
}

FightClub_Trolls_L12_15_V3 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Trolls_Ranks_01.Minions,
        ["Encounter_S_32"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_01"] = Trolls_Ranks_01.Boss,
        ["Encounter_E_02"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_03"] = Trolls_Ranks_01.Boss,
        ["Encounter_E_04"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_05"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_08"] = Trolls_Ranks_01.Boss,
    },
}

FightClub_Trolls_L16_19_V0 = FightClub_Trolls_L12_15_V0
FightClub_Trolls_L16_19_V1 = FightClub_Trolls_L12_15_V1
FightClub_Trolls_L16_19_V2 = FightClub_Trolls_L12_15_V2
FightClub_Trolls_L16_19_V3 = FightClub_Trolls_L12_15_V3


-- Vandalism --

Vandalism_Trolls_L11_13_V0 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_02"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_05"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_06"] = Trolls_Ranks_01.Minions,                     
    },
}

Vandalism_Trolls_L11_13_V1 = Vandalism_Trolls_L11_13_V0
Vandalism_Trolls_L11_13_V2 = Vandalism_Trolls_L11_13_V0
Vandalism_Trolls_L11_13_V3 = Vandalism_Trolls_L11_13_V0

Vandalism_Trolls_L14_17_V0 = Vandalism_Trolls_L11_13_V0
Vandalism_Trolls_L14_17_V1 = Vandalism_Trolls_L11_13_V0
Vandalism_Trolls_L14_17_V2 = Vandalism_Trolls_L11_13_V0
Vandalism_Trolls_L14_17_V3 = Vandalism_Trolls_L11_13_V0

Vandalism_Trolls_L18_20_V0 = Vandalism_Trolls_L11_13_V0
Vandalism_Trolls_L18_20_V1 = Vandalism_Trolls_L11_13_V0
Vandalism_Trolls_L18_20_V2 = Vandalism_Trolls_L11_13_V0
Vandalism_Trolls_L18_20_V3 = Vandalism_Trolls_L11_13_V0

-- Breaking in --
BreakingIn_Trolls_L11_13_V0 = {
    ["Markers"] = {
        ["Encounter_S_31"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_05"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_06"] = Trolls_Ranks_01.Minions,                    
    },
}

BreakingIn_Trolls_L11_13_V1 = BreakingIn_Trolls_L11_13_V0
BreakingIn_Trolls_L11_13_V2 = BreakingIn_Trolls_L11_13_V0
BreakingIn_Trolls_L11_13_V3 = BreakingIn_Trolls_L11_13_V0
BreakingIn_Trolls_L11_13_V4 = BreakingIn_Trolls_L11_13_V0

BreakingIn_Trolls_L14_17_V0 = BreakingIn_Trolls_L11_13_V0
BreakingIn_Trolls_L14_17_V1 = BreakingIn_Trolls_L11_13_V0 
BreakingIn_Trolls_L14_17_V2 = BreakingIn_Trolls_L11_13_V0
BreakingIn_Trolls_L14_17_V3 = BreakingIn_Trolls_L11_13_V0
BreakingIn_Trolls_L14_17_V4 = BreakingIn_Trolls_L11_13_V0

BreakingIn_Trolls_L18_20_V0 = BreakingIn_Trolls_L11_13_V0
BreakingIn_Trolls_L18_20_V1 = BreakingIn_Trolls_L11_13_V0
BreakingIn_Trolls_L18_20_V2 = BreakingIn_Trolls_L11_13_V0
BreakingIn_Trolls_L18_20_V3 = BreakingIn_Trolls_L11_13_V0
BreakingIn_Trolls_L18_20_V4 = BreakingIn_Trolls_L11_13_V0

-- Peddle / Mugging / Snatch --

Peddle_Trolls_L12_15_V0 = {
    ["Markers"] = {
        ["Encounter_V_40"] = Trolls_Ranks_01.Victims,
        ["Encounter_S_30"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_05"] = Trolls_Ranks_01.Minions,
        ["Encounter_S_31"] = Trolls_Ranks_01.Minions,
    },
}
Peddle_Trolls_L12_15_V1 = Peddle_Trolls_L12_15_V0
Peddle_Trolls_L12_15_V2 = Peddle_Trolls_L12_15_V0

Peddle_Trolls_L16_19_V0 = Peddle_Trolls_L12_15_V0
Peddle_Trolls_L16_19_V1 = Peddle_Trolls_L12_15_V0
Peddle_Trolls_L16_19_V2 = Peddle_Trolls_L12_15_V0
