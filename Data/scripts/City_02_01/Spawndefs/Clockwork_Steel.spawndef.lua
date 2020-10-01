--This is a rank table
--There could be multiple tables to generate spawns from
local Clockwork_Ranks_01  = {
    ["Underlings"] = {
            "Sprocket_Mini","Sprocket_00", "Sprocket_01", "Sprocket_02",
    },
    ["Minions"] = {
            "Sprocket_09", "Sprocket_091", "Sprocket_092",
            "Sprocket_093", "Sprocket_094",
            "Sprocket_10", "Sprocket_11",
            "Sprocket_12", "Sprocket_19",
            "Sprocket_191", "Sprocket_192",
            "Sprocket_193", "Sprocket_194",
    },
    ["Flying"] = {
            "Oscillator_00",
    },
    ["Lieutenants"] = {
            "Sprocket_21", "Sprocket_22", "Sprocket_23"
    },
    ["Sniper"] = {
      --NA
    },
    ["Boss"] = {
            "Diesel_00"
    },
    ["Elite Boss"] = {
      --NA
    },
    ["Victims"] = {

    },
    ["Specials"] = {

    },
}

--== SCAVENGING ==--

Scavenge_Clockwork_D1_V0 = {
    ["Markers"] = {
            ["Encounter_E_02"] = Clockwork_Ranks_01.Minions,
            ["Encounter_E_05"] = Clockwork_Ranks_01.Minions,
            ["Encounter_E_07"] = Clockwork_Ranks_01.Minions,
            ["Encounter_E_08"] = Clockwork_Ranks_01.Minions,
    },
}

Scavenge_Clockwork_D1_V1 = {
    ["Markers"] = {
            ["Encounter_E_05"] = Clockwork_Ranks_01.Minions,
            ["Encounter_E_07"] = Clockwork_Ranks_01.Minions,
            ["Encounter_E_02"] = Clockwork_Ranks_01.Minions,
            ["Encounter_E_08"] = Clockwork_Ranks_01.Minions,                                
    },
}

Scavenge_Clockwork_D1_V2 = {
    ["Markers"] = {
        ["Encounter_E_05"] = Clockwork_Ranks_01.Minions,
        ["Encounter_E_07"] = Clockwork_Ranks_01.Minions,
        ["Encounter_E_01"] = Clockwork_Ranks_01.Minions,
        ["Encounter_E_04"] = Clockwork_Ranks_01.Minions,
        ["Encounter_E_02"] = Clockwork_Ranks_01.Minions,
        ["Encounter_S_30"] = Clockwork_Ranks_01.Minions,                        
    },
}

Scavenge_Clockwork_D1_V3 = {
    ["Markers"] = {
        ["Encounter_E_01"] = Clockwork_Ranks_01.Minions,
        ["Encounter_E_03"] = Clockwork_Ranks_01.Minions,
        ["Encounter_E_04"] = Clockwork_Ranks_01.Minions,
        ["Encounter_E_06"] = Clockwork_Ranks_01.Minions,
        ["Encounter_S_30"] = Clockwork_Ranks_01.Minions,
        ["Encounter_S_31"] = Clockwork_Ranks_01.Minions,
    },
}

Scavenge_Clockwork_D5_V0 = Scavenge_Clockwork_D1_V0
Scavenge_Clockwork_D5_V1 = Scavenge_Clockwork_D1_V1
Scavenge_Clockwork_D5_V2 = Scavenge_Clockwork_D1_V2
Scavenge_Clockwork_D5_V3 = Scavenge_Clockwork_D1_V3

Scavenge_Clockwork_D9_V0 = Scavenge_Clockwork_D1_V0
Scavenge_Clockwork_D9_V1 = Scavenge_Clockwork_D1_V1
Scavenge_Clockwork_D9_V2 = Scavenge_Clockwork_D1_V2
Scavenge_Clockwork_D9_V3 = Scavenge_Clockwork_D1_V3