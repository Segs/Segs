--This is a rank table
--There could be multiple tables to generate spawns from
local BanishedPantheon_Ranks_01  = {
    ["Underlings"] = {
        "Pantheon_Husk_01",	"Pantheon_Husk_02",
        "Pantheon_Husk_03", "Pantheon_Husk_04",
        "Pantheon_Husk_05", "Pantheon_Husk_06",
        "Pantheon_Husk_07", "Pantheon_Husk_08",
        "Pantheon_Husk_09", "Pantheon_Husk_10",
        "Pantheon_Husk_11", "Pantheon_Husk_12",
        "Pantheon_Husk_13", "Pantheon_Husk_14",
        "Pantheon_Husk_15"
    },
    ["Minions"] = { 
        "Pantheon_Hull_01", "Pantheon_Hull_02",
        "Pantheon_Hull_03", "Pantheon_Hull_04",
        "Pantheon_Hull_05", "Pantheon_Hull_06",
        "Pantheon_Hull_07", "Pantheon_Hull_08",
        "Pantheon_Hull_09"
    },
    ["Lieutenants"] = { 
        "Pantheon_Witchdoctor_01", "Pantheon_Witchdoctor_02",
        "Pantheon_Witchdoctor_03", "Pantheon_Witchdoctor_04",
        "Pantheon_Witchdoctor_05", "Pantheon_Witchdoctor_06",
    },
    ["Boss"] = {
        "Pantheon_Sorrow", "Pantheon_Pain",
        "Pantheon_Desire", "Pantheon_Death",
        "Pantheon_Totem" 
    },
}


-- Loiter --

Loiter_BanishedPantheon_D5_V0 = {
    ["Markers"] = {
        ["Encounter_S_31"] = BanishedPantheon_Ranks_01.Underlings,
        ["Encounter_S_30"] = BanishedPantheon_Ranks_01.Underlings,
        ["Encounter_E_06"] = BanishedPantheon_Ranks_01.Underlings,
        ["Encounter_E_05"] = BanishedPantheon_Ranks_01.Boss,
    },
}

Loiter_BanishedPantheon_D5_V1 = {
    ["Markers"] = {
        ["Encounter_S_31"] = BanishedPantheon_Ranks_01.Minions,
        ["Encounter_S_30"] = BanishedPantheon_Ranks_01.Minions,
        ["Encounter_E_02"] = BanishedPantheon_Ranks_01.Underlings,
        ["Encounter_E_01"] = BanishedPantheon_Ranks_01.Minions,
        ["Encounter_E_08"] = BanishedPantheon_Ranks_01.Minions,
        ["Encounter_E_07"] = BanishedPantheon_Ranks_01.Lieutenants,        
    },
}

Loiter_BanishedPantheon_D5_V2 = {
    ["Markers"] = {
        ["Encounter_S_31"] = BanishedPantheon_Ranks_01.Lieutenants,
        ["Encounter_S_30"] = BanishedPantheon_Ranks_01.Minions,
        ["Encounter_E_06"] = BanishedPantheon_Ranks_01.Minions,
        ["Encounter_E_05"] = BanishedPantheon_Ranks_01.Minions,
        ["Encounter_S_31"] = BanishedPantheon_Ranks_01.Minions,
        ["Encounter_E_02"] = BanishedPantheon_Ranks_01.Minions,     
    },
}

Loiter_BanishedPantheon_D5_V3 = {
    ["Markers"] = {
        ["Encounter_E_03"] = BanishedPantheon_Ranks_01.Underlings,
        ["Encounter_E_04"] = BanishedPantheon_Ranks_01.Underlings,
        ["Encounter_E_02"] = BanishedPantheon_Ranks_01.Underlings,
        ["Encounter_E_01"] = BanishedPantheon_Ranks_01.Underlings,
        ["Encounter_E_07"] = BanishedPantheon_Ranks_01.Boss,
    },
}

Loiter_BanishedPantheon_D5_V4 = {
    ["Markers"] = {
        ["Encounter_S_31"] = BanishedPantheon_Ranks_01.Underlings,
        ["Encounter_S_30"] = BanishedPantheon_Ranks_01.Minions,
        ["Encounter_E_03"] = BanishedPantheon_Ranks_01.Underlings,
        ["Encounter_E_04"] = BanishedPantheon_Ranks_01.Minions,
        ["Encounter_E_07"] = BanishedPantheon_Ranks_01.Minions,
        ["Encounter_E_08"] = BanishedPantheon_Ranks_01.Lieutenants,        
    },
}

Loiter_BanishedPantheon_D10_V0 = Loiter_BanishedPantheon_D5_V0
Loiter_BanishedPantheon_D10_V1 = Loiter_BanishedPantheon_D5_V1
Loiter_BanishedPantheon_D10_V2 = Loiter_BanishedPantheon_D5_V2
Loiter_BanishedPantheon_D10_V3 = Loiter_BanishedPantheon_D5_V3
Loiter_BanishedPantheon_D10_V4 = Loiter_BanishedPantheon_D5_V4