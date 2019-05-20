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
        "Behemoth_Lord", "Behemoth_Envoy"
    },
    ["Elite Boss"] = {
        "CoTUniqueThreatLevel1", "CoTUniqueThreatLevel2"
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

local Warriors_Rumble_01 = {
    ["Minions"] = {
        "Thug_Warrior_01", "Thug_Warrior_02", "Thug_Warrior_03",
        "Thug_Warrior_04", "Thug_Warrior_05", "Thug_Warrior_06",
    },
    ["Boss"] = {
        "Thug_Warrior_Boss_01", "Thug_Warrior_Boss_02", "Thug_Warrior_Boss_03",
    },
}

-- Loiter --

Loiter_CoT_E_D1_V0 = {
    ["Markers"] = {
            ["Encounter_S_32"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,            
   },
}

Loiter_CoT_E_D1_V1 = Loiter_CoT_E_D1_V0
Loiter_CoT_E_D1_V2 = Loiter_CoT_E_D1_V0
Loiter_CoT_E_D1_V3 = Loiter_CoT_E_D1_V0
Loiter_CoT_E_D1_V4 = Loiter_CoT_E_D1_V0

Loiter_CoT_MH_D5_V0 = Loiter_CoT_E_D1_V0
Loiter_CoT_MH_D5_V1 = Loiter_CoT_E_D1_V0
Loiter_CoT_MH_D5_V2 = Loiter_CoT_E_D1_V0
Loiter_CoT_MH_D5_V3 = Loiter_CoT_E_D1_V0
Loiter_CoT_MH_D5_V4 = Loiter_CoT_E_D1_V0

Loiter_CoT_MH_D10_V0 = Loiter_CoT_E_D1_V0
Loiter_CoT_MH_D10_V1 = Loiter_CoT_E_D1_V0
Loiter_CoT_MH_D10_V2 = Loiter_CoT_E_D1_V0
Loiter_CoT_MH_D10_V3 = Loiter_CoT_E_D1_V0
Loiter_CoT_MH_D10_V4 = Loiter_CoT_E_D1_V0


-- Sacrifice --

Sacrifice_CoT_E_D1_V0 = {
    ["Markers"] = {
        ["Encounter_S_32"] = Circle_Ranks_01.Lieutenants,
        ["Encounter_E_04"] = Circle_Ranks_01.Minions,
        ["Encounter_E_03"] = Circle_Ranks_01.Minions,
        ["Encounter_E_01"] = Circle_Ranks_01.Minions,
        ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
        ["Encounter_E_02"] = Circle_Ranks_01.Minions,                
},
}

Sacrifice_CoT_E_D1_V1 = Sacrifice_CoT_E_D1_V0
Sacrifice_CoT_E_D1_V2 = Sacrifice_CoT_E_D1_V0
Sacrifice_CoT_E_D1_V3 = Sacrifice_CoT_E_D1_V0
Sacrifice_CoT_E_D1_V4 = Sacrifice_CoT_E_D1_V0

Sacrifice_CoT_MH_D5_V0 = Sacrifice_CoT_E_D1_V0
Sacrifice_CoT_MH_D5_V1 = Sacrifice_CoT_E_D1_V0
Sacrifice_CoT_MH_D5_V2 = Sacrifice_CoT_E_D1_V0
Sacrifice_CoT_MH_D5_V3 = Sacrifice_CoT_E_D1_V0
Sacrifice_CoT_MH_D5_V4 = Sacrifice_CoT_E_D1_V0

Sacrifice_CoT_MH_D10_V0 = Sacrifice_CoT_E_D1_V0
Sacrifice_CoT_MH_D10_V1 = Sacrifice_CoT_E_D1_V0
Sacrifice_CoT_MH_D10_V2 = Sacrifice_CoT_E_D1_V0
Sacrifice_CoT_MH_D10_V3 = Sacrifice_CoT_E_D1_V0
Sacrifice_CoT_MH_D10_V4 = Sacrifice_CoT_E_D1_V0

-- Rumble --

Rumble_CoTWarriors_D3_V0 = {
    ["Markers"] = {
        ["Encounter_E_09"] = Circle_Ranks_01.Minions,
        ["Encounter_E_03"] = Circle_Ranks_01.Boss,
        ["Encounter_E_01"] = Circle_Ranks_01.Minions,
        ["Encounter_E_05"] = Circle_Ranks_01.Minions,
        ["Encounter_S_33"] = Circle_Ranks_01.Lieutenants,
        ["Encounter_S_31"] = Circle_Ranks_01.Minions,
        ["Encounter_E_02"] = Warriors_Rumble_01.Minions,
        ["Encounter_E_10"] = Warriors_Rumble_01.Minions,
        ["Encounter_E_04"] = Warriors_Rumble_01.Minions,
        ["Encounter_E_06"] = Warriors_Rumble_01.Boss,
        ["Encounter_S_32"] = Warriors_Rumble_01.Lieutenants,
        ["Encounter_S_30"] = Warriors_Rumble_01.Minions,           
   },
}

Rumble_CoTWarriors_D3_V1 = Rumble_CoTWarriors_D3_V0
Rumble_CoTWarriors_D3_V2 = Rumble_CoTWarriors_D3_V0
Rumble_CoTWarriors_D3_V3 = Rumble_CoTWarriors_D3_V0
Rumble_CoTWarriors_D3_V4 = Rumble_CoTWarriors_D3_V0

Rumble_CoTWarriors_D8_V0 = Rumble_CoTWarriors_D3_V0
Rumble_CoTWarriors_D8_V1 = Rumble_CoTWarriors_D3_V0
Rumble_CoTWarriors_D8_V2 = Rumble_CoTWarriors_D3_V0
Rumble_CoTWarriors_D8_V3 = Rumble_CoTWarriors_D3_V0
Rumble_CoTWarriors_D8_V4 = Rumble_CoTWarriors_D3_V0