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
    },
    ["Victims"] = {
        "FemaleNPC_01", "FemaleNPC_02", "FemaleNPC_03",
        "FemaleNPC_11", "FemaleNPC_12", "MaleNPC_01",
        "MaleNPC_02", "MaleNPC_03", "MaleNPC_04",
        "MaleNPC_15", "MaleNPC_16",
    },  
    ["Specials"] = {

    },
}

Sacrifice_CoT_D3_V0 = {
    ["Markers"] = {
            ["Encounter_V_40"] = Circle_Ranks_01.Victims,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
    },
}

Sacrifice_CoT_D3_V1 = {
    ["Markers"] = {
            ["Encounter_V_40"] = Circle_Ranks_01.Victims,       
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_01"] = Circle_Ranks_01.Lieutenants,                
    },
}

Sacrifice_CoT_D3_V2 = {
    ["Markers"] = {
            ["Encounter_V_40"] = Circle_Ranks_01.Victims,       
            ["Encounter_E_01"] = Circle_Ranks_01.Minions,
            ["Encounter_E_02"] = Circle_Ranks_01.Minions,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_S_30"] = Circle_Ranks_01.Minions,
            ["Encounter_S_32"] = Circle_Ranks_01.Minions,                
    },
}

Sacrifice_CoT_D3_V3 = {
    ["Markers"] = {
            ["Encounter_V_40"] = Circle_Ranks_01.Victims,               
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Lieutenants,
    },
}

Sacrifice_CoT_D3_V4 = {
    ["Markers"] = {
            ["Encounter_V_40"] = Circle_Ranks_01.Victims,
            ["Encounter_S_30"] = Circle_Ranks_01.Lieutenants,
            ["Encounter_E_05"] = Circle_Ranks_01.Minions,
            ["Encounter_E_07"] = Circle_Ranks_01.Minions,
            ["Encounter_E_01"] = Circle_Ranks_01.Lieutenants,                
    },
}

Sacrifice_CoT_D8_V0 = Sacrifice_CoT_D3_V0
Sacrifice_CoT_D8_V1 = Sacrifice_CoT_D3_V1
Sacrifice_CoT_D8_V2 = Sacrifice_CoT_D3_V2
Sacrifice_CoT_D8_V3 = Sacrifice_CoT_D3_V3
Sacrifice_CoT_D8_V4 = Sacrifice_CoT_D3_V4
