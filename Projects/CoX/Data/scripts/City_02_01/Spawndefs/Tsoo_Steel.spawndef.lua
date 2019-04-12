local Tsoo_Ranks_01  = {
    ["Underlings"] = {
      --NA
    },
    ["Minions"] = {
        "Tsoo_01", "Tsoo_02", "Tsoo_03",
        "Tsoo_04", "Tsoo_05", "Tsoo_06",
        "Tsoo_07", "Tsoo_08", "Tsoo_09",
        "Tsoo_10", "Tsoo_11", "Tsoo_12",
        "Tsoo_13", "Tsoo_14", "Tsoo_15",
        "Tsoo_16", "Tsoo_17",
    },
    ["Lieutenants"] = {
        "Inkman_01", "Inkman_02", "Inkman_03", "Inkman_04",
        "Inkman_05", "Inkman_06",
    },
    ["Sniper"] = {
      --NA
    },
    ["Boss"] = {
        "Mystic_01", "Mystic_02", "Mystic_03", "Mystic_04",
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

--== THREATEN ==--

Threaten_TsooOutcasts_D3_V0 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Tsoo_Ranks_01.Victims,
            ["Encounter_S_30"] = Tsoo_Ranks_01.Minions,
            ["Encounter_E_07"] = Tsoo_Ranks_01.Minions,
            ["Encounter_E_08"] = Tsoo_Ranks_01.Minions,
    },
}

Threaten_TsooOutcasts_D3_V1 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Tsoo_Ranks_01.Victims,
            ["Encounter_S_30"] = Tsoo_Ranks_01.Lieutenants,
            ["Encounter_E_07"] = Tsoo_Ranks_01.Minions,
    },
}

Threaten_TsooOutcasts_D3_V2 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Tsoo_Ranks_01.Victims,
            ["Encounter_S_30"] = Tsoo_Ranks_01.Lieutenants,
            ["Encounter_E_05"] = Tsoo_Ranks_01.Minions,
            ["Encounter_E_07"] = Tsoo_Ranks_01.Minions,
            ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,
    },
}

Threaten_TsooOutcasts_D8_V0 = Threaten_TsooOutcasts_D3_V0
Threaten_TsooOutcasts_D8_V1 = Threaten_TsooOutcasts_D3_V1
Threaten_TsooOutcasts_D8_V2 = Threaten_TsooOutcasts_D3_V2