--This is a rank table
--There could be multiple tables to generate spawns from
local Warriors_Ranks_01  = {
    ["Underlings"] = {
      --NA
    },
    ["Minions"] = {
        "Thug_Warrior_01", "Thug_Warrior_02", "Thug_Warrior_03",
        "Thug_Warrior_04", "Thug_Warrior_05", "Thug_Warrior_06",
    },
    ["Lieutenants"] = {
        "Thug_Warrior_01", "Thug_Warrior_02", "Thug_Warrior_03",
        "Thug_Warrior_04", "Thug_Warrior_05", "Thug_Warrior_06",
    },
    ["Sniper"] = {
      --NA
    },
    ["Boss"] = {
        "Thug_Warrior_Boss_01", "Thug_Warrior_Boss_02", "Thug_Warrior_Boss_03",
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
    ["Specials"] = {

    },
    ["Tsoo_Victims"] = {
        "Tsoo_01","Tsoo_02","Tsoo_03",
        "Tsoo_04","Tsoo_05","Tsoo_06",
        "Tsoo_07","Tsoo_08","Tsoo_09",
        "Tsoo_10","Tsoo_11","Tsoo_12",
        "Tsoo_13","Tsoo_14","Tsoo_15",
        "Tsoo_16","Tsoo_17",
    }
}

ShadyDeal_Warriors_D1_V0 = {
    ["Markers"] = {
        ["Encounter_E_06"] = Warriors_Ranks_01.Minions,
        ["Encounter_E_04"] = Warriors_Ranks_01.Minions,
        ["Encounter_E_03"] = Warriors_Ranks_01.Minions,
        ["Encounter_S_30"] = Warriors_Ranks_01.Minions,
        ["Encounter_V_40"] = Warriors_Ranks_01.Minions,
    },
}

ShadyDeal_Warriors_D1_V1 = ShadyDeal_Warriors_D1_V0
ShadyDeal_Warriors_D1_V2 = ShadyDeal_Warriors_D1_V0
ShadyDeal_Warriors_D1_V3 = ShadyDeal_Warriors_D1_V0
ShadyDeal_Warriors_D1_V4 = ShadyDeal_Warriors_D1_V0

ShadyDeal_Warriors_D5_V0 = ShadyDeal_Warriors_D1_V0
ShadyDeal_Warriors_D5_V1 = ShadyDeal_Warriors_D1_V0
ShadyDeal_Warriors_D5_V2 = ShadyDeal_Warriors_D1_V0
ShadyDeal_Warriors_D5_V3 = ShadyDeal_Warriors_D1_V0
ShadyDeal_Warriors_D5_V4 = ShadyDeal_Warriors_D1_V0

ShadyDeal_Warriors_D10_V0 = ShadyDeal_Warriors_D1_V0
ShadyDeal_Warriors_D10_V1 = ShadyDeal_Warriors_D1_V0
ShadyDeal_Warriors_D10_V2 = ShadyDeal_Warriors_D1_V0
ShadyDeal_Warriors_D10_V3 = ShadyDeal_Warriors_D1_V0
ShadyDeal_Warriors_D10_V4 = ShadyDeal_Warriors_D1_V0

FightClub_Warriors_D1_V0 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Warriors_Ranks_01.Boss,
        ["Encounter_S_32"] = Warriors_Ranks_01.Boss,
        ["Encounter_E_01"] = Warriors_Ranks_01.Boss,
        ["Encounter_E_02"] = Warriors_Ranks_01.Minions,
        ["Encounter_E_04"] = Warriors_Ranks_01.Minions,
        ["Encounter_E_06"] = Warriors_Ranks_01.Minions,
        ["Encounter_E_07"] = Warriors_Ranks_01.Minions,
    },
}

FightClub_Warriors_D1_V1 = FightClub_Warriors_D1_V0
FightClub_Warriors_D1_V2 = FightClub_Warriors_D1_V0
FightClub_Warriors_D1_V3 = FightClub_Warriors_D1_V0

FightClub_Warriors_D5_V0 = FightClub_Warriors_D1_V0
FightClub_Warriors_D5_V1 = FightClub_Warriors_D1_V0
FightClub_Warriors_D5_V2 = FightClub_Warriors_D1_V0
FightClub_Warriors_D5_V3 = FightClub_Warriors_D1_V0

FightClub_Warriors_D10_V0 = FightClub_Warriors_D1_V0
FightClub_Warriors_D10_V1 = FightClub_Warriors_D1_V0
FightClub_Warriors_D10_V2 = FightClub_Warriors_D1_V0
FightClub_Warriors_D10_V3 = FightClub_Warriors_D1_V0

PurseSnatch_Tsoo_D3_V0 = {
    ["Markers"] = {
        ["Encounter_E_05"] = Warriors_Ranks_01.Minions,
        ["Encounter_E_03"] = Warriors_Ranks_01.Minions,
        ["Encounter_S_30"] = Warriors_Ranks_01.Boss,
        ["Encounter_V_40"] = Warriors_Ranks_01.Tsoo_Victims,
    },
}

PurseSnatch_Tsoo_D3_V1 = PurseSnatch_Tsoo_D3_V0
PurseSnatch_Tsoo_D3_V2 = PurseSnatch_Tsoo_D3_V0

PurseSnatch_Tsoo_D8_V0 = PurseSnatch_Tsoo_D3_V0
PurseSnatch_Tsoo_D8_V1 = PurseSnatch_Tsoo_D3_V0
PurseSnatch_Tsoo_D8_V2 = PurseSnatch_Tsoo_D3_V0