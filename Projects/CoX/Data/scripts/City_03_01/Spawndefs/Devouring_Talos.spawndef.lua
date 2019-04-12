--This is a rank table
--There could be multiple tables to generate spawns from

-- Not sure on all of these models
local DevouringEarth_Ranks_01  = {
    ["Underlings"] = {
        "Devouring_Swarm",
    },
    ["Minions"] = {
        "Mushroom_Beast_01","Mushroom_Beast_02","Plant_Beast_01",
        "Plant_Beast_02","Plant_Beast_03","Plant_Beast_04",
    },
    ["Lieutenants"] = {
        "Mushroom_Beast_03","Plant_Beast_05",
    },
    ["Sniper"] = {
      --NA
    },
    ["Boss"] = {
        "Devoured_01","Devoured_02","Devoured_03",
        "Devoured_04","Devoured_05",
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

-- Loiter --

Loiter_DevouringEarth_D10_V0 = {
    ["Markers"] = {
            ["Encounter_S_32"] = DevouringEarth_Ranks_01.Minions,
            ["Encounter_E_03"] = DevouringEarth_Ranks_01.Minions,
            ["Encounter_E_07"] = DevouringEarth_Ranks_01.Minions,            
   },
}

Loiter_DevouringEarth_D10_V1 = {
    ["Markers"] = {
            ["Encounter_E_07"] = DevouringEarth_Ranks_01.Boss,            
   },
}

Loiter_DevouringEarth_D10_V2 = {
    ["Markers"] = {
            ["Encounter_E_05"] = DevouringEarth_Ranks_01.Boss,
            ["Encounter_S_30"] = DevouringEarth_Ranks_01.Minions,
            ["Encounter_E_07"] = DevouringEarth_Ranks_01.Minions,
            ["Encounter_E_08"] = DevouringEarth_Ranks_01.Minions,             
   },
}

Loiter_DevouringEarth_D10_V3 = {
    ["Markers"] = {
            ["Encounter_E_04"] = DevouringEarth_Ranks_01.Boss,
            ["Encounter_S_30"] = DevouringEarth_Ranks_01.Minions,
            ["Encounter_E_05"] = DevouringEarth_Ranks_01.Minions,
            ["Encounter_E_08"] = DevouringEarth_Ranks_01.Minions,
            ["Encounter_S_31"] = DevouringEarth_Ranks_01.Boss,
            ["Encounter_E_07"] = DevouringEarth_Ranks_01.Minions,
            ["Encounter_E_01"] = DevouringEarth_Ranks_01.Minions,                           
   },
}

Loiter_DevouringEarth_D10_V4 = {
    ["Markers"] = {
            ["Encounter_E_04"] = DevouringEarth_Ranks_01.Minions,
            ["Encounter_E_03"] = DevouringEarth_Ranks_01.Minions,
            ["Encounter_S_30"] = DevouringEarth_Ranks_01.Minions,
            ["Encounter_E_05"] = DevouringEarth_Ranks_01.Minions,
            ["Encounter_E_08"] = DevouringEarth_Ranks_01.Minions,
            ["Encounter_S_31"] = DevouringEarth_Ranks_01.Lieutenants,
            ["Encounter_E_07"] = DevouringEarth_Ranks_01.Lieutenants,
            ["Encounter_E_01"] = DevouringEarth_Ranks_01.Boss,                           
   },
}

Planting_DevouringEarth_D10_V0 = Loiter_DevouringEarth_D10_V0
Planting_DevouringEarth_D10_V1 = Loiter_DevouringEarth_D10_V0
Planting_DevouringEarth_D10_V2 = Loiter_DevouringEarth_D10_V0
Planting_DevouringEarth_D10_V3 = Loiter_DevouringEarth_D10_V0
Planting_DevouringEarth_D10_V4 = Loiter_DevouringEarth_D10_V0