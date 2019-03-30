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
        "MaleNPC_90", "MaleNPC_91", "MaleNPC_92",
        "MaleNPC_93", "MaleNPC_94", "MaleNPC_95",
        "MaleNPC_96", "MaleNPC_97", "MaleNPC_98",
        "MaleNPC_99",
    },
    ["Specials"] = {

    },
}

local Fifth_Rumble_01  = {
    ["Minions"] = {
        "5thNight_00", "5thNight_01",
    },
    ["Lieutenants"] = {
        "5thUber_40",
    },
}

local Freaks_Rumble_01  = {
    ["Minions"] = {
        "FRK_01","FRK_02","FRK_03","FRK_04",
        "FRK_05","FRK_06","FRK_07",
    },
    ["Lieutenants"] = {
        "FRK_14",
    },
}

--==RUMBLE: Tsoo vs. Fifth ==--

Rumble_Tsoo5thColumn_D3_V0 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_02"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_07"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,
   },
}

Rumble_Tsoo5thColumn_D3_V1 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_02"] = Fifth_Rumble_01.Minions,
        ["Encounter_S_34"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_07"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_35"] = Tsoo_Ranks_01.Minions,
   },
}

Rumble_Tsoo5thColumn_D3_V2 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_02"] = Fifth_Rumble_01.Minions,
        ["Encounter_S_34"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_06"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_07"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_35"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_31"] = Tsoo_Ranks_01.Minions,
   },
}

Rumble_Tsoo5thColumn_D3_V3 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_02"] = Fifth_Rumble_01.Minions,
        ["Encounter_S_34"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_06"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_10"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_12"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_11"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_35"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_07"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_35"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_31"] = Tsoo_Ranks_01.Minions,
   },
}

Rumble_Tsoo5thColumn_D3_V4 = {
    ["Markers"] = {
        ["Encounter_E_10"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_04"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_12"] = Fifth_Rumble_01.Minions,
        ["Encounter_S_32"] = Fifth_Rumble_01.Minions,
        ["Encounter_S_30"] = Fifth_Rumble_01.Minions,        
        ["Encounter_S_33"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_03"] = Tsoo_Ranks_01.Minions,        
        ["Encounter_E_11"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_09"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_35"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_07"] = Tsoo_Ranks_01.Minions,
   },
}

Rumble_Tsoo5thColumn_D8_V0 = Rumble_Tsoo5thColumn_D3_V0
Rumble_Tsoo5thColumn_D8_V1 = Rumble_Tsoo5thColumn_D3_V1
Rumble_Tsoo5thColumn_D8_V2 = Rumble_Tsoo5thColumn_D3_V2
Rumble_Tsoo5thColumn_D8_V3 = Rumble_Tsoo5thColumn_D3_V3
Rumble_Tsoo5thColumn_D8_V4 = Rumble_Tsoo5thColumn_D3_V4

--==RUMBLE: Tsoo vs. Freaks ==--

Rumble_FreakshowTsoo_D5_V0 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Freaks_Rumble_01.Minions,
        ["Encounter_E_02"] = Freaks_Rumble_01.Minions,
        ["Encounter_E_07"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,
   },
}

Rumble_FreakshowTsoo_D5_V1 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Freaks_Rumble_01.Minions,
        ["Encounter_E_02"] = Freaks_Rumble_01.Minions,
        ["Encounter_S_30"] = Freaks_Rumble_01.Minions,
        ["Encounter_S_31"] = Tsoo_Ranks_01.Minions,        
        ["Encounter_E_07"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,
   },
}

Rumble_FreakshowTsoo_D5_V2 = Rumble_FreakshowTsoo_D5_V0
Rumble_FreakshowTsoo_D5_V3 = Rumble_FreakshowTsoo_D5_V0
Rumble_FreakshowTsoo_D5_V4 = Rumble_FreakshowTsoo_D5_V0

--== AROUND VANDALISM ==--

Vandalism_Tsoo_D1_V0  = {
    ["Markers"] = {
        ["Encounter_S_30"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_08"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_04"] = Tsoo_Ranks_01.Lieutenants,
   },
}

Vandalism_Tsoo_D1_V1  = {
    ["Markers"] = {
        ["Encounter_S_30"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_08"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_04"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_E_05"] = Tsoo_Ranks_01.Lieutenants,
   },
}

Vandalism_Tsoo_D1_V2  = {
    ["Markers"] = {
        ["Encounter_S_30"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_08"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_04"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_S_31"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_09"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_07"] = Tsoo_Ranks_01.Lieutenants,        
   },
}

Vandalism_Tsoo_D1_V3  = {
    ["Markers"] = {
        ["Encounter_S_30"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_08"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_04"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_S_31"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_09"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_07"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_E_06"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_01"] = Tsoo_Ranks_01.Minions,         
   },
}

Vandalism_Tsoo_D5_V0 = Vandalism_Tsoo_D1_V0
Vandalism_Tsoo_D5_V1 = Vandalism_Tsoo_D1_V1
Vandalism_Tsoo_D5_V2 = Vandalism_Tsoo_D1_V2
Vandalism_Tsoo_D5_V3 = Vandalism_Tsoo_D1_V3

Vandalism_Tsoo_D10_V0 = Vandalism_Tsoo_D1_V0
Vandalism_Tsoo_D10_V1 = Vandalism_Tsoo_D1_V1
Vandalism_Tsoo_D10_V2 = Vandalism_Tsoo_D1_V2
Vandalism_Tsoo_D10_V3 = Vandalism_Tsoo_D1_V3

--== MUGGING ==--

Mugging_Tsoo_D1_V0  = {
    ["Markers"] = {
        ["Encounter_V_40"] = Tsoo_Ranks_01.Victims,
        ["Encounter_S_30"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_08"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_04"] = Tsoo_Ranks_01.Lieutenants,
   },
}

Mugging_Tsoo_D1_V1  = {
    ["Markers"] = {
        ["Encounter_V_40"] = Tsoo_Ranks_01.Victims,
        ["Encounter_S_30"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_32"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_08"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_04"] = Tsoo_Ranks_01.Lieutenants,
   },
}

Mugging_Tsoo_D1_V2  = {
    ["Markers"] = {
        ["Encounter_V_40"] = Tsoo_Ranks_01.Victims,
        ["Encounter_V_42"] = Tsoo_Ranks_01.Victims,
        ["Encounter_S_30"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_31"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_08"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_E_04"] = Tsoo_Ranks_01.Lieutenants,
   },
}

Mugging_Tsoo_D1_V3  = {
    ["Markers"] = {
        ["Encounter_V_40"] = Tsoo_Ranks_01.Victims,
        ["Encounter_V_42"] = Tsoo_Ranks_01.Victims,
        ["Encounter_S_30"] = Tsoo_Ranks_01.Minions,
        ["Encounter_S_31"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_02"] = Tsoo_Ranks_01.Minions,
        ["Encounter_E_05"] = Tsoo_Ranks_01.Lieutenants,
        ["Encounter_E_07"] = Tsoo_Ranks_01.Lieutenants,
   },
}

Mugging_Tsoo_D5_V0 = Mugging_Tsoo_D1_V0
Mugging_Tsoo_D5_V1 = Mugging_Tsoo_D1_V1
Mugging_Tsoo_D5_V2 = Mugging_Tsoo_D1_V2
Mugging_Tsoo_D5_V3 = Mugging_Tsoo_D1_V3

Mugging_Tsoo_D10_V0 = Mugging_Tsoo_D1_V0
Mugging_Tsoo_D10_V1 = Mugging_Tsoo_D1_V1
Mugging_Tsoo_D10_V2 = Mugging_Tsoo_D1_V2
Mugging_Tsoo_D10_V3 = Mugging_Tsoo_D1_V3
