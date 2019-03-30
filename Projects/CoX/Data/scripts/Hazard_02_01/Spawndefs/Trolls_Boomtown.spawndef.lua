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
    ["Lieutenants"] = {

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

Loiter_Trolls_L11_13_V0 = {
    ["Markers"] = {
        ["Encounter_S_32"] = Trolls_Ranks_01.Boss,
        ["Encounter_S_30"] = Trolls_Ranks_01.Boss,
        ["Encounter_E_03"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_07"] = Trolls_Ranks_01.Minions,            
    },
}

Loiter_Trolls_L18_20_V0 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Trolls_Ranks_01.Minions,
            ["Encounter_S_32"] = Trolls_Ranks_01.Minions,
            ["Encounter_E_01"] = Trolls_Ranks_01.Boss,
            ["Encounter_E_04"] = Trolls_Ranks_01.Boss,
    },
}

FightClub_Trolls_L12_15_V0 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Trolls_Ranks_01.Minions,
        ["Encounter_S_32"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_05"] = Trolls_Ranks_01.Boss,
        ["Encounter_E_06"] = Trolls_Ranks_01.Boss,
        ["Encounter_E_07"] = Trolls_Ranks_01.Minions,
    },
}

FightClub_Trolls_L16_19_V0 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Trolls_Ranks_01.Minions,
        ["Encounter_S_32"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_01"] = Trolls_Ranks_01.Boss,
        ["Encounter_E_02"] = Trolls_Ranks_01.Minions,
        ["Encounter_E_03"] = Trolls_Ranks_01.Boss,
    },
}
