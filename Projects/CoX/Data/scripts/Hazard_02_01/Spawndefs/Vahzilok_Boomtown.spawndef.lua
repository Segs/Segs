--This is a rank table
--There could be multiple tables to generate spawns from
local Vahzilok_Ranks_01  = {
    ["Underlings"] = {
      --NA
    },
    ["Minions"] = {
        "Reaper_01", "Reaper_02", "Reaper_03",
        "Reaper_04", "Reaper_05", "Reaper_06",
        "Reaper_07", "Reaper_08", "Reaper_09",
        "Reaper_10", "Reaper_11", "Reaper_12",
        "Reaper_13", "Reaper_14", "Reaper_15",
        "Reaper_16",
    },
    ["Lieutenants"] = {
        "Eidola_Male", "Eidola_Female",
    },
    ["Sniper"] = {
      --NA
    },
    ["Boss"] = {
      --NA
    },
    ["Elite Boss"] = {
      --NA
    },
    ["Victims"] = {
      "MaleNPC_100", "MaleNPC_103", "MaleNPC_105",
      "MaleNPC_101", "MaleNPC_104", "MaleNPC_106",
      "MaleNPC_102",
  },
    ["Specials"] = {

    },
}

--== HARVEST ==--

Harvest_Vahzilok_L11_13_V0 = {
    ["Markers"] = {
        ["Encounter_S_31"] = Vahzilok_Ranks_01.Lieutenants,
        ["Encounter_S_30"] = Vahzilok_Ranks_01.Minions,
        ["Encounter_E_07"] = Vahzilok_Ranks_01.Minions,
    },
}

Harvest_Vahzilok_L14_17_V0 = {
  ["Markers"] = {
        ["Encounter_V_42"] = Vahzilok_Ranks_01.Victims,
        ["Encounter_S_32"] = Vahzilok_Ranks_01.Lieutenants,
        ["Encounter_E_03"] = Vahzilok_Ranks_01.Minions,
        ["Encounter_E_07"] = Vahzilok_Ranks_01.Minions,             
 },
}

Harvest_Vahzilok_L18_20_V0 = {
  ["Markers"] = {
        ["Encounter_V_42"] = Vahzilok_Ranks_01.Victims,
        ["Encounter_V_40"] = Vahzilok_Ranks_01.Victims,
        ["Encounter_S_32"] = Vahzilok_Ranks_01.Lieutenants,
        ["Encounter_E_03"] = Vahzilok_Ranks_01.Minions,
        ["Encounter_E_01"] = Vahzilok_Ranks_01.Minions,
        ["Encounter_S_31"] = Vahzilok_Ranks_01.Lieutenants,            
 },
}

--== LOITER ==--

Loiter_Vahzilok_L12_15_V0 = Harvest_Vahzilok_L14_17_V0
Loiter_Vahzilok_L16_19_V0 = Harvest_Vahzilok_L18_20_V0