--This is a rank table
--There could be multiple tables to generate spawns from
local Vahzilok_Ranks_01  = {
    ["Underlings"] = {
      --NA
    },
    ["Minions"] = {
            "Slab_01", "Slab_02", "Slab_03",
            "Slab_04", "Cylok_01", "Cylok_02",
            "Cylok_03", "Cylok_04", "Cylok_Slab_01",
            "Cylok_Slab_02", "Cylok_Slab_03", "Cylok_Slab_04",
    },
    ["Lieutenants"] = {
            "Reaper_01", "Reaper_02", "Reaper_03",
            "Reaper_04", "Reaper_05", "Reaper_06",
            "Reaper_07", "Reaper_08", "Reaper_09",
            "Reaper_10", "Reaper_11", "Reaper_12",
            "Reaper_13", "Reaper_14", "Reaper_15",
            "Reaper_16",
    },
    ["Sniper"] = {
      --NA
    },
    ["Boss"] = {
            "Eidola_Male", "Eidola_Female"
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

--== HARVEST ==--

Vahzilok_Harvest_D3_V0 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Vahzilok_Ranks_01.Victims,
            ["Encounter_S_32"] = Vahzilok_Ranks_01.Lieutenants,
            ["Encounter_E_03"] = Vahzilok_Ranks_01.Minions,
            ["Encounter_E_07"] = Vahzilok_Ranks_01.Minions,            
   },
}

Vahzilok_Harvest_D3_V1 = Vahzilok_Harvest_D3_V0
Vahzilok_Harvest_D3_V2 = Vahzilok_Harvest_D3_V0

Vahzilok_Harvest_D8_V0 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Vahzilok_Ranks_01.Victims,
            ["Encounter_V_40"] = Vahzilok_Ranks_01.Victims,
            ["Encounter_S_32"] = Vahzilok_Ranks_01.Lieutenants,
            ["Encounter_E_03"] = Vahzilok_Ranks_01.Minions,
            ["Encounter_E_07"] = Vahzilok_Ranks_01.Minions,
            ["Encounter_E_01"] = Vahzilok_Ranks_01.Minions,
            ["Encounter_S_31"] = Vahzilok_Ranks_01.Lieutenants,            
   },
}

Vahzilok_Harvest_D8_V1 = Vahzilok_Harvest_D8_V0
Vahzilok_Harvest_D8_V2 = Vahzilok_Harvest_D8_V0


--== HUNTING ==--

Vahzilok_Hunting_D3_V0 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Vahzilok_Ranks_01.Victims,        
            ["Encounter_S_32"] = Vahzilok_Ranks_01.Lieutenants,
            ["Encounter_S_30"] = Vahzilok_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Vahzilok_Ranks_01.Lieutenants,            
   },
}

Vahzilok_Hunting_D3_V1 = Vahzilok_Hunting_D3_V0
Vahzilok_Hunting_D3_V2 = Vahzilok_Hunting_D3_V0

Vahzilok_Hunting_D8_V0 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Vahzilok_Ranks_01.Victims,
            ["Encounter_V_40"] = Vahzilok_Ranks_01.Victims,
            ["Encounter_S_32"] = Vahzilok_Ranks_01.Lieutenants,
            ["Encounter_S_30"] = Vahzilok_Ranks_01.Lieutenants,
            ["Encounter_E_07"] = Vahzilok_Ranks_01.Boss,
            ["Encounter_S_31"] = Vahzilok_Ranks_01.Boss,            
   },
}

Vahzilok_Hunting_D8_V1 = Vahzilok_Hunting_D8_V0
Vahzilok_Hunting_D8_V2 = Vahzilok_Hunting_D8_V0