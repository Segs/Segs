local Tsoo_Ranks_01  = {
    ["Underlings"] = {
      "Tsoo_01", "Tsoo_02", "Tsoo_03",
        "Tsoo_04", "Tsoo_05", "Tsoo_06",
        "Tsoo_07", "Tsoo_08", "Tsoo_09",
        "Tsoo_10", "Tsoo_11", "Tsoo_12",
        "Tsoo_13", "Tsoo_14", "Tsoo_15",
        "Tsoo_16", "Tsoo_17",
    },
    ["Minions"] = {
        "Inkman_01", "Inkman_02", "Inkman_03", "Inkman_04",
        "Inkman_05", "Inkman_06",
    },
    ["Lieutenants"] = {
        "Tsoo_Spirit", "Pha_01", "Pha_02",
        "Pha_03", "Pha_04", "Pha_05"
    },
    ["Boss"] = {
        "Mystic_01", "Mystic_02", "Mystic_03", "Mystic_04",
    },
}

Investigate_Tsoo_D1_V0 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_30"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_S_31"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_S_32"] = Tsoo_Ranks_01.Boss
    },
}


Investigate_Tsoo_D1_V1 = {
    ["Markers"] = {
        ["Encounter_E_01"] = Tsoo_Ranks_01.Underlings,
        ["Encounter_E_02"] = Tsoo_Ranks_01.Underlings,
        ["Encounter_E_03"] = Tsoo_Ranks_01.Underlings,
        ["Encounter_S_30"] = Tsoo_Ranks_01.Lieutenants,
    },
}

Investigate_Tsoo_D1_V2 = {
    ["Markers"] = {
        ["Encounter_E_04"] = Tsoo_Ranks_01.Underlings,
        ["Encounter_E_05"] = Tsoo_Ranks_01.Underlings,
        ["Encounter_E_06"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_07"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_31"] = Tsoo_Ranks_01.Boss,
        ["Encounter_S_32"] = Tsoo_Ranks_01.Lieutenants
    },
}

Investigate_Tsoo_D5_V0 = Investigate_Tsoo_D1_V0
Investigate_Tsoo_D5_V1 = Investigate_Tsoo_D1_V1
Investigate_Tsoo_D5_V2 = Investigate_Tsoo_D1_V2

Investigate_Tsoo_D10_V0 = Investigate_Tsoo_D1_V0
Investigate_Tsoo_D10_V1 = Investigate_Tsoo_D1_V1
Investigate_Tsoo_D10_V2 = Investigate_Tsoo_D1_V2