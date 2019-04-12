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

--== CEREMONY ==--

CircleOfThorns_Ceremony_D3_V0 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Circle_Ranks_01.Victims,
            ["Encounter_S_32"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,            
   },
}

CircleOfThorns_Ceremony_D3_V1 = CircleOfThorns_Ceremony_D3_V0
CircleOfThorns_Ceremony_D3_V2 = CircleOfThorns_Ceremony_D3_V0

CircleOfThorns_Ceremony_D8_V0 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Circle_Ranks_01.Victims,
            ["Encounter_S_32"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Minions,
            ["Encounter_E_03"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,            
   },
}

CircleOfThorns_Ceremony_D8_V1 = CircleOfThorns_Ceremony_D8_V0
CircleOfThorns_Ceremony_D8_V2 = CircleOfThorns_Ceremony_D8_V0