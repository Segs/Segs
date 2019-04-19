--This is a rank table
--There could be multiple tables to generate spawns from
local Family_Ranks_01  = {
    ["Minions"] = {
        "Thug_Family_01","Thug_Family_02","Thug_Family_03","Thug_Family_04",
        "Thug_Family_05","Thug_Family_06",
    },
    ["Lieutenants"] = {
        "Thug_Family_Boss_01", "Thug_Family_Boss_02",
    },
    ["Victims"] = {
        "MaleNPC_100", "MaleNPC_101", "MaleNPC_102", "MaleNPC_103",
        "MaleNPC_104", "MaleNPC_105", "MaleNPC_106", "MaleNPC_107",
        "MaleNPC_108", "MaleNPC_109", "MaleNPC_110",
    }
}

local Tsoo_Rumble_01  = {
    ["Minions"] = {
        "Tsoo_01", "Tsoo_02", "Tsoo_03",
        "Tsoo_04", "Tsoo_05", "Tsoo_06",
        "Tsoo_07", "Tsoo_08", "Tsoo_09",
    },
    ["Lieutenants"] = {
        "Inkman_01", "Inkman_02", "Inkman_03", "Inkman_04",
        "Inkman_05", "Inkman_06",
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

--== FIGHT CLUB: Family ==--

FightClub_Family_D1_V0 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Family_Ranks_01.Minions,
            ["Encounter_S_32"] = Family_Ranks_01.Minions,
            ["Encounter_E_01"] = Family_Ranks_01.Minions,
    },
}

FightClub_Family_D1_V1 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Family_Ranks_01.Minions,
            ["Encounter_S_32"] = Family_Ranks_01.Minions,
            ["Encounter_E_01"] = Family_Ranks_01.Lieutenants,
            ["Encounter_E_04"] = Family_Ranks_01.Lieutenants,
    },
}

FightClub_Family_D1_V2 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Family_Ranks_01.Minions,
        ["Encounter_S_32"] = Family_Ranks_01.Minions,
        ["Encounter_E_01"] = Family_Ranks_01.Lieutenants,
        ["Encounter_E_04"] = Family_Ranks_01.Lieutenants,
        ["Encounter_E_05"] = Family_Ranks_01.Minions,
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
    },
}

FightClub_Family_D1_V3 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Family_Ranks_01.Lieutenants,
        ["Encounter_S_32"] = Family_Ranks_01.Lieutenants,
        ["Encounter_E_01"] = Family_Ranks_01.Lieutenants,
        ["Encounter_E_02"] = Family_Ranks_01.Minions,
        ["Encounter_E_04"] = Family_Ranks_01.Minions,
        ["Encounter_E_06"] = Family_Ranks_01.Minions,
        ["Encounter_E_07"] = Family_Ranks_01.Minions,
    },
}

FightClub_Family_D5_V0 = FightClub_Family_D1_V0
FightClub_Family_D5_V1 = FightClub_Family_D1_V1
FightClub_Family_D5_V2 = FightClub_Family_D1_V2
FightClub_Family_D5_V3 = FightClub_Family_D1_V3

FightClub_Family_D10_V0 = FightClub_Family_D1_V0
FightClub_Family_D10_V1 = FightClub_Family_D1_V1
FightClub_Family_D10_V2 = FightClub_Family_D1_V2
FightClub_Family_D10_V3 = FightClub_Family_D1_V3

--== LOITERING ==--

Loiter_Family_D1_V0 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Family_Ranks_01.Minions,
            ["Encounter_S_32"] = Family_Ranks_01.Minions,
            ["Encounter_E_01"] = Family_Ranks_01.Minions,
    },
}

Loiter_Family_D1_V1 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Family_Ranks_01.Minions,
            ["Encounter_S_32"] = Family_Ranks_01.Minions,
            ["Encounter_E_01"] = Family_Ranks_01.Lieutenants,
            ["Encounter_E_04"] = Family_Ranks_01.Lieutenants,
    },
}

Loiter_Family_D1_V2 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Family_Ranks_01.Minions,
        ["Encounter_S_32"] = Family_Ranks_01.Minions,
        ["Encounter_E_01"] = Family_Ranks_01.Lieutenants,
        ["Encounter_E_04"] = Family_Ranks_01.Lieutenants,
        ["Encounter_E_05"] = Family_Ranks_01.Minions,
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
    },
}

Loiter_Family_D1_V3 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Family_Ranks_01.Lieutenants,
        ["Encounter_S_32"] = Family_Ranks_01.Lieutenants,
        ["Encounter_E_01"] = Family_Ranks_01.Minions,
        ["Encounter_E_02"] = Family_Ranks_01.Minions,
        ["Encounter_E_04"] = Family_Ranks_01.Minions,
        ["Encounter_E_05"] = Family_Ranks_01.Minions,
        ["Encounter_E_07"] = Family_Ranks_01.Minions,
    },
}

Loiter_Family_D5_V0 = Loiter_Family_D1_V0
Loiter_Family_D5_V1 = Loiter_Family_D1_V1
Loiter_Family_D5_V2 = Loiter_Family_D1_V2
Loiter_Family_D5_V3 = Loiter_Family_D1_V3

Loiter_Family_D10_V0 = Loiter_Family_D1_V0
Loiter_Family_D10_V1 = Loiter_Family_D1_V1
Loiter_Family_D10_V2 = Loiter_Family_D1_V2
Loiter_Family_D10_V3 = Loiter_Family_D1_V3

--== SHADY DEALS ==--

ShadyDeal_Family_D1_V0 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Family_Ranks_01.Minions,
            ["Encounter_S_32"] = Family_Ranks_01.Minions,
            ["Encounter_E_01"] = Family_Ranks_01.Minions,
    },
}

ShadyDeal_Family_D1_V1 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Family_Ranks_01.Lieutenants,
            ["Encounter_S_32"] = Family_Ranks_01.Lieutenants,
            ["Encounter_E_01"] = Family_Ranks_01.Minions,
            ["Encounter_E_04"] = Family_Ranks_01.Minions,
    },
}

ShadyDeal_Family_D1_V2 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Family_Ranks_01.Lieutenants,
        ["Encounter_S_32"] = Family_Ranks_01.Lieutenants,
        ["Encounter_E_01"] = Family_Ranks_01.Minions,
        ["Encounter_E_04"] = Family_Ranks_01.Minions,
        ["Encounter_E_05"] = Family_Ranks_01.Minions,
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
    },
}

ShadyDeal_Family_D1_V3 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Family_Ranks_01.Lieutenants,
        ["Encounter_S_32"] = Family_Ranks_01.Lieutenants,
        ["Encounter_E_02"] = Family_Ranks_01.Minions,
        ["Encounter_E_03"] = Family_Ranks_01.Minions,
        ["Encounter_E_05"] = Family_Ranks_01.Minions,
        ["Encounter_E_06"] = Family_Ranks_01.Minions,
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
    },
}

ShadyDeal_Family_D5_V0 = ShadyDeal_Family_D1_V0
ShadyDeal_Family_D5_V1 = ShadyDeal_Family_D1_V1
ShadyDeal_Family_D5_V2 = ShadyDeal_Family_D1_V2
ShadyDeal_Family_D5_V3 = ShadyDeal_Family_D1_V3

ShadyDeal_Family_D10_V0 = ShadyDeal_Family_D1_V0
ShadyDeal_Family_D10_V1 = ShadyDeal_Family_D1_V1
ShadyDeal_Family_D10_V2 = ShadyDeal_Family_D1_V2
ShadyDeal_Family_D10_V3 = ShadyDeal_Family_D1_V3

--== SHAKE DOWNS ==--

Shakedown_Family_D1_V0 = {
    ["Markers"] = {
        ["Encounter_V_40"] = Family_Ranks_01.Victims,
        ["Encounter_S_30"] = Family_Ranks_01.Minions,
        ["Encounter_S_32"] = Family_Ranks_01.Minions,
        ["Encounter_E_01"] = Family_Ranks_01.Minions,
    },
}

Shakedown_Family_D1_V1 = {
    ["Markers"] = {
        ["Encounter_V_40"] = Family_Ranks_01.Victims,        
        ["Encounter_S_30"] = Family_Ranks_01.Lieutenants,
        ["Encounter_S_32"] = Family_Ranks_01.Lieutenants,
        ["Encounter_E_01"] = Family_Ranks_01.Minions,
        ["Encounter_E_04"] = Family_Ranks_01.Minions,
    },
}

Shakedown_Family_D1_V2 = {
    ["Markers"] = {
        ["Encounter_V_40"] = Family_Ranks_01.Victims,
        ["Encounter_V_42"] = Family_Ranks_01.Victims,        
        ["Encounter_S_30"] = Family_Ranks_01.Lieutenants,
        ["Encounter_E_01"] = Family_Ranks_01.Minions,
        ["Encounter_E_04"] = Family_Ranks_01.Minions,
        ["Encounter_E_05"] = Family_Ranks_01.Minions,
    },
}

Shakedown_Family_D1_V3 = {
    ["Markers"] = {
        ["Encounter_V_40"] = Family_Ranks_01.Victims,
        ["Encounter_V_42"] = Family_Ranks_01.Victims,
        ["Encounter_S_30"] = Family_Ranks_01.Lieutenants,
        ["Encounter_S_32"] = Family_Ranks_01.Lieutenants,
        ["Encounter_E_06"] = Family_Ranks_01.Minions,
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
    },
}

Shakedown_Family_D1_V4 = {
    ["Markers"] = {
        ["Encounter_V_40"] = Family_Ranks_01.Victims,
        ["Encounter_V_41"] = Family_Ranks_01.Victims,
        ["Encounter_V_42"] = Family_Ranks_01.Victims,
        ["Encounter_S_30"] = Family_Ranks_01.Lieutenants,
        ["Encounter_S_32"] = Family_Ranks_01.Lieutenants,
        ["Encounter_E_05"] = Family_Ranks_01.Minions,
        ["Encounter_E_06"] = Family_Ranks_01.Minions,
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
    },
}

Shakedown_Family_D5_V0 = Shakedown_Family_D1_V0
Shakedown_Family_D5_V1 = Shakedown_Family_D1_V1
Shakedown_Family_D5_V2 = Shakedown_Family_D1_V2
Shakedown_Family_D5_V3 = Shakedown_Family_D1_V3
Shakedown_Family_D5_V4 = Shakedown_Family_D1_V4

Shakedown_Family_D10_V0 = Shakedown_Family_D1_V0
Shakedown_Family_D10_V1 = Shakedown_Family_D1_V1
Shakedown_Family_D10_V2 = Shakedown_Family_D1_V2
Shakedown_Family_D10_V3 = Shakedown_Family_D1_V3
Shakedown_Family_D10_V4 = Shakedown_Family_D1_V4

--==RUMBLE: Tsoo vs. Family ==--

Rumble_FamilyTsoo_D3_V0 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
        ["Encounter_E_02"] = Family_Ranks_01.Minions,
        ["Encounter_E_07"] = Tsoo_Rumble_01.Minions,
        ["Encounter_E_01"] = Tsoo_Rumble_01.Minions,
   },
}

Rumble_FamilyTsoo_D3_V1 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
        ["Encounter_E_02"] = Family_Ranks_01.Minions,
        ["Encounter_S_34"] = Family_Ranks_01.Minions,
        ["Encounter_E_07"] = Tsoo_Rumble_01.Minions,
        ["Encounter_E_01"] = Tsoo_Rumble_01.Minions,
        ["Encounter_S_35"] = Tsoo_Rumble_01.Minions,
   },
}

Rumble_FamilyTsoo_D3_V2 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
        ["Encounter_E_02"] = Family_Ranks_01.Minions,
        ["Encounter_S_34"] = Family_Ranks_01.Minions,
        ["Encounter_E_06"] = Family_Ranks_01.Minions,
        ["Encounter_E_07"] = Tsoo_Rumble_01.Minions,
        ["Encounter_E_01"] = Tsoo_Rumble_01.Minions,
        ["Encounter_S_35"] = Tsoo_Rumble_01.Minions,
        ["Encounter_S_31"] = Tsoo_Rumble_01.Minions,
   },
}

Rumble_FamilyTsoo_D3_V3 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
        ["Encounter_E_02"] = Family_Ranks_01.Minions,
        ["Encounter_S_34"] = Family_Ranks_01.Minions,
        ["Encounter_E_06"] = Family_Ranks_01.Minions,
        ["Encounter_E_10"] = Family_Ranks_01.Minions,
        ["Encounter_S_35"] = Tsoo_Rumble_01.Minions,
        ["Encounter_E_07"] = Tsoo_Rumble_01.Minions,
        ["Encounter_E_01"] = Tsoo_Rumble_01.Minions,
        ["Encounter_S_35"] = Tsoo_Rumble_01.Minions,
        ["Encounter_S_31"] = Tsoo_Rumble_01.Minions,
   },
}

Rumble_FamilyTsoo_D3_V4 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
        ["Encounter_E_02"] = Family_Ranks_01.Minions,
        ["Encounter_S_34"] = Family_Ranks_01.Minions,
        ["Encounter_E_06"] = Family_Ranks_01.Minions,
        ["Encounter_E_10"] = Family_Ranks_01.Minions,
        ["Encounter_E_04"] = Family_Ranks_01.Minions,
        ["Encounter_E_12"] = Family_Ranks_01.Minions,
        ["Encounter_E_11"] = Tsoo_Rumble_01.Minions,
        ["Encounter_E_09"] = Tsoo_Rumble_01.Minions,
        ["Encounter_S_33"] = Tsoo_Rumble_01.Minions,
        ["Encounter_E_07"] = Tsoo_Rumble_01.Minions,
        ["Encounter_E_01"] = Tsoo_Rumble_01.Minions,
        ["Encounter_S_35"] = Tsoo_Rumble_01.Minions,
        ["Encounter_S_31"] = Tsoo_Rumble_01.Minions,
   },
}

Rumble_FamilyTsoo_D8_V0 = Rumble_FamilyTsoo_D3_V0
Rumble_FamilyTsoo_D8_V1 = Rumble_FamilyTsoo_D3_V1
Rumble_FamilyTsoo_D8_V2 = Rumble_FamilyTsoo_D3_V2
Rumble_FamilyTsoo_D8_V3 = Rumble_FamilyTsoo_D3_V3
Rumble_FamilyTsoo_D8_V4 = Rumble_FamilyTsoo_D3_V4

--==RUMBLE: Fifth vs. Family ==--

Rumble_5thFamily_D3_V0 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
        ["Encounter_E_06"] = Family_Ranks_01.Minions,        
        ["Encounter_E_02"] = Family_Ranks_01.Minions,
        ["Encounter_E_07"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_01"] = Fifth_Rumble_01.Minions,
        ["Encounter_E_03"] = Fifth_Rumble_01.Minions,
   },
}

Rumble_5thFamily_D3_V1 = Rumble_5thFamily_D3_V0
Rumble_5thFamily_D3_V2 = Rumble_5thFamily_D3_V0
Rumble_5thFamily_D3_V3 = Rumble_5thFamily_D3_V0
Rumble_5thFamily_D3_V4 = Rumble_5thFamily_D3_V0

Rumble_5thFamily_D8_V0 = Rumble_5thFamily_D3_V0
Rumble_5thFamily_D8_V1 = Rumble_5thFamily_D3_V0
Rumble_5thFamily_D8_V2 = Rumble_5thFamily_D3_V0
Rumble_5thFamily_D8_V3 = Rumble_5thFamily_D3_V0
Rumble_5thFamily_D8_V4 = Rumble_5thFamily_D3_V0

--==RUMBLE: Freaks vs. Family ==--

Rumble_FreakshowFamily_D5_V0 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
        ["Encounter_E_02"] = Family_Ranks_01.Minions,
        ["Encounter_E_07"] = Freaks_Rumble_01.Minions,
        ["Encounter_E_01"] = Freaks_Rumble_01.Minions,
   },
}

Rumble_FreakshowFamily_D5_V1 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
        ["Encounter_E_02"] = Family_Ranks_01.Minions,
        ["Encounter_S_34"] = Family_Ranks_01.Minions,
        ["Encounter_E_07"] = Freaks_Rumble_01.Minions,
        ["Encounter_E_01"] = Freaks_Rumble_01.Minions,
        ["Encounter_S_35"] = Freaks_Rumble_01.Minions,
   },
}

Rumble_FreakshowFamily_D5_V2 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
        ["Encounter_E_02"] = Family_Ranks_01.Minions,
        ["Encounter_S_34"] = Family_Ranks_01.Minions,
        ["Encounter_E_06"] = Family_Ranks_01.Minions,
        ["Encounter_E_07"] = Freaks_Rumble_01.Minions,
        ["Encounter_E_01"] = Freaks_Rumble_01.Minions,
        ["Encounter_S_35"] = Freaks_Rumble_01.Minions,
        ["Encounter_S_31"] = Freaks_Rumble_01.Minions,
   },
}

Rumble_FreakshowFamily_D5_V3 = {
    ["Markers"] = {
        ["Encounter_E_08"] = Family_Ranks_01.Minions,
        ["Encounter_E_02"] = Family_Ranks_01.Minions,
        ["Encounter_S_34"] = Family_Ranks_01.Minions,
        ["Encounter_E_06"] = Family_Ranks_01.Minions,
        ["Encounter_E_10"] = Family_Ranks_01.Minions,
        ["Encounter_E_12"] = Family_Ranks_01.Minions,
        ["Encounter_E_11"] = Freaks_Rumble_01.Minions,
        ["Encounter_S_33"] = Freaks_Rumble_01.Minions,
        ["Encounter_E_07"] = Freaks_Rumble_01.Minions,
        ["Encounter_E_01"] = Freaks_Rumble_01.Minions,
        ["Encounter_S_35"] = Freaks_Rumble_01.Minions,
        ["Encounter_S_31"] = Freaks_Rumble_01.Minions,
   },
}

Rumble_FreakshowFamily_D5_V4 = {
    ["Markers"] = {
        ["Encounter_E_10"] = Family_Ranks_01.Minions,
        ["Encounter_E_12"] = Family_Ranks_01.Minions,
        ["Encounter_S_34"] = Family_Ranks_01.Minions,
        ["Encounter_E_06"] = Family_Ranks_01.Minions,
        ["Encounter_E_11"] = Freaks_Rumble_01.Minions,
        ["Encounter_E_01"] = Freaks_Rumble_01.Minions,
        ["Encounter_S_33"] = Freaks_Rumble_01.Minions,
        ["Encounter_S_31"] = Freaks_Rumble_01.Minions,
   },
}
