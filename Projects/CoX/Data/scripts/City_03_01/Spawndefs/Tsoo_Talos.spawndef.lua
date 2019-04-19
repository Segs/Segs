--This is a rank table
--There could be multiple tables to generate spawns from
local Tsoo_Ranks_01  = {
    ["Underlings"] = {
      --NA
    },
    ["Minions"] = {
        "Tsoo_01","Tsoo_04","Tsoo_05",
        "Tsoo_06","Tsoo_08","Tsoo_09",
        "Tsoo_12","Tsoo_17",
    },
    ["Lieutenants"] = {
        "Tsoo_10","Tsoo_14","Tsoo_15","Tsoo_Spirit",
        "Mystic_01","Mystic_02","Mystic_03","Mystic_04",
    },
    ["Sniper"] = {
      --NA
    },
    ["Boss"] = {
        "Tsoo_03","Tsoo_13","Tsoo_16","Tsoo_07","Tsoo_11","Tsoo_02","Tsoo_09",
    },
    ["Elite Boss"] = {
      --NA
    },
    ["Victims"] = {
        
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


--== Shady Deal ==---
ShadyDeal_Tsoo_D1_V0 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Tsoo_Ranks_01.Minions,
            ["Encounter_S_32"] = Tsoo_Ranks_01.Minions,
            ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,
    },
}

ShadyDeal_Tsoo_D1_V1 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Tsoo_Ranks_01.Boss,
            ["Encounter_S_32"] = Tsoo_Ranks_01.Lieutenants,
            ["Encounter_E_02"] = Tsoo_Ranks_01.Minions,
            ["Encounter_E_04"] = Tsoo_Ranks_01.Minions,
    },
}

ShadyDeal_Tsoo_D1_V2 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_S_31"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_03"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_05"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_E_08"] = Tsoo_Ranks_01.Minions,
    },
}

ShadyDeal_Tsoo_D1_V3 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Tsoo_Ranks_01.Boss,
        ["Encounter_S_32"] = Tsoo_Ranks_01.Boss,
        ["Encounter_E_02"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_03"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_05"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_07"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_E_08"] = Tsoo_Ranks_01.Lieutenants,
    },
}

ShadyDeal_Tsoo_D1_V4 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_S_32"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_02"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_04"] = Tsoo_Ranks_01.Boss,
        ["Encounter_E_06"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_08"] = Tsoo_Ranks_01.Boss,
    },
}

ShadyDeal_Tsoo_D5_V0 = ShadyDeal_Tsoo_D1_V0
ShadyDeal_Tsoo_D5_V1 = ShadyDeal_Tsoo_D1_V1
ShadyDeal_Tsoo_D5_V2 = ShadyDeal_Tsoo_D1_V2
ShadyDeal_Tsoo_D5_V3 = ShadyDeal_Tsoo_D1_V3
ShadyDeal_Tsoo_D5_V4 = ShadyDeal_Tsoo_D1_V4

ShadyDeal_Tsoo_D10_V0 = ShadyDeal_Tsoo_D1_V0
ShadyDeal_Tsoo_D10_V1 = ShadyDeal_Tsoo_D1_V1
ShadyDeal_Tsoo_D10_V2 = ShadyDeal_Tsoo_D1_V2
ShadyDeal_Tsoo_D10_V3 = ShadyDeal_Tsoo_D1_V3
ShadyDeal_Tsoo_D10_V4 = ShadyDeal_Tsoo_D1_V4

-- Loiter --

Loiter_Tsoo_D1_V0 = {
    ["Markers"] = {
        ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_02"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_03"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_06"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_30"] = Tsoo_Ranks_01.Lieutenants,
    },
}
 
Loiter_Tsoo_D1_V1 = Loiter_Tsoo_D1_V0
Loiter_Tsoo_D1_V2 = Loiter_Tsoo_D1_V0
Loiter_Tsoo_D1_V3 = Loiter_Tsoo_D1_V0
Loiter_Tsoo_D1_V4 = Loiter_Tsoo_D1_V0

Loiter_Tsoo_D5_V0 = Loiter_Tsoo_D1_V0
Loiter_Tsoo_D5_V1 = Loiter_Tsoo_D1_V0
Loiter_Tsoo_D5_V2 = Loiter_Tsoo_D1_V0
Loiter_Tsoo_D5_V3 = Loiter_Tsoo_D1_V0
Loiter_Tsoo_D5_V4 = Loiter_Tsoo_D1_V0

Loiter_Tsoo_D10_V0 = Loiter_Tsoo_D1_V0
Loiter_Tsoo_D10_V1 = Loiter_Tsoo_D1_V0
Loiter_Tsoo_D10_V2 = Loiter_Tsoo_D1_V0
Loiter_Tsoo_D10_V3 = Loiter_Tsoo_D1_V0
Loiter_Tsoo_D10_V4 = Loiter_Tsoo_D1_V0


-- Shakedown --

Shakedown_Tsoo_D1_V0 = {
    ["Markers"] = {
        ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_04"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_E_06"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_30"] = Tsoo_Ranks_01.Boss,
    },
}

Shakedown_Tsoo_D1_V1 = Shakedown_Tsoo_D1_V0
Shakedown_Tsoo_D1_V2 = Shakedown_Tsoo_D1_V0
Shakedown_Tsoo_D1_V3 = Shakedown_Tsoo_D1_V0
Shakedown_Tsoo_D1_V4 = Shakedown_Tsoo_D1_V0

Shakedown_Tsoo_D5_V0 = Shakedown_Tsoo_D1_V0
Shakedown_Tsoo_D5_V1 = Shakedown_Tsoo_D1_V0
Shakedown_Tsoo_D5_V2 = Shakedown_Tsoo_D1_V0
Shakedown_Tsoo_D5_V3 = Shakedown_Tsoo_D1_V0
Shakedown_Tsoo_D5_V4 = Shakedown_Tsoo_D1_V0

Shakedown_Tsoo_D10_V0 = Shakedown_Tsoo_D1_V0
Shakedown_Tsoo_D10_V1 = Shakedown_Tsoo_D1_V0
Shakedown_Tsoo_D10_V2 = Shakedown_Tsoo_D1_V0
Shakedown_Tsoo_D10_V3 = Shakedown_Tsoo_D1_V0
Shakedown_Tsoo_D10_V4 = Shakedown_Tsoo_D1_V0

-- Rumble --
Rumble_TsooWarriors_D3_V0 = {
    ["Markers"] = {
        ["Encounter_E_09"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_03"] = Tsoo_Ranks_01.Boss,
        ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_05"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_33"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_31"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_02"] = Warriors_Rumble_01.Minions,
        ["Encounter_E_10"] = Warriors_Rumble_01.Minions,
        ["Encounter_E_04"] = Warriors_Rumble_01.Minions,
        ["Encounter_E_06"] = Warriors_Rumble_01.Boss,
        ["Encounter_S_32"] = Warriors_Rumble_01.Minions,
        ["Encounter_S_30"] = Warriors_Rumble_01.Minions,           
   },
}

Rumble_TsooWarriors_D3_V1 = Rumble_TsooWarriors_D3_V0
Rumble_TsooWarriors_D3_V2 = Rumble_TsooWarriors_D3_V0
Rumble_TsooWarriors_D3_V3 = Rumble_TsooWarriors_D3_V0
Rumble_TsooWarriors_D3_V4 = Rumble_TsooWarriors_D3_V0

Rumble_TsooWarriors_D8_V0 = Rumble_TsooWarriors_D3_V0
Rumble_TsooWarriors_D8_V1 = Rumble_TsooWarriors_D3_V0
Rumble_TsooWarriors_D8_V2 = Rumble_TsooWarriors_D3_V0
Rumble_TsooWarriors_D8_V3 = Rumble_TsooWarriors_D3_V0
Rumble_TsooWarriors_D8_V4 = Rumble_TsooWarriors_D3_V0