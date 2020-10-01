--This is a rank table
--There could be multiple tables to generate spawns from
local Skulls_Ranks_01  = {
    ["Underlings"] = {
      --NA
    },
    ["Minions"] = {
            "Thug_Skull_01", "Thug_Skull_02", "Thug_Skull_03",
            "Thug_Skull_04", "Thug_Skull_05", "Thug_Skull_06",
    },
    ["Lieutenants"] = {
            "Thug_Skull_01", "Thug_Skull_02", "Thug_Skull_03",
            "Thug_Skull_04", "Thug_Skull_05", "Thug_Skull_06",
    },
    ["Sniper"] = {
      --NA
    },
    ["Boss"] = {
            "Thug_Skull_Boss_01", "Thug_Skull_Boss_02", "Thug_Skull_Boss_03",
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

local Vahzilok_Rumble_01  = {
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
    ["Boss"] = {
        "Eidola_Male", "Eidola_Female",
    }
}

--== MUGGINGS ==--

Skulls_Mugging_D1_V0 = {
    ["Markers"] = {
            ["Encounter_V_42"] = Skulls_Ranks_01.Victims,
            ["Encounter_S_32"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_03"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_07"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_01"] = Skulls_Ranks_01.Minions,            
   },
}

Skulls_Mugging_D1_V1 = Skulls_Mugging_D1_V0
Skulls_Mugging_D1_V2 = Skulls_Mugging_D1_V0
Skulls_Mugging_D1_V3 = Skulls_Mugging_D1_V0
Skulls_ShadyDeal_D1_V0 = Skulls_Mugging_D1_V0
Skulls_ShadyDeal_D1_V1 = Skulls_Mugging_D1_V0
Skulls_ShadyDeal_D1_V2 = Skulls_Mugging_D1_V0


Skulls_Mugging_D5_V0 = {
    ["Markers"] = {
            ["Encounter_V_40"] = Skulls_Ranks_01.Victims,
            ["Encounter_V_42"] = Skulls_Ranks_01.Victims,
            ["Encounter_S_32"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_03"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_04"] = Skulls_Ranks_01.Lieutenants,
            ["Encounter_E_01"] = Skulls_Ranks_01.Lieutenants,            
   },
}

Skulls_Mugging_D5_V1 = Skulls_Mugging_D5_V0
Skulls_Mugging_D5_V2 = Skulls_Mugging_D5_V0
Skulls_Mugging_D5_V3 = Skulls_Mugging_D5_V0
Skulls_ShadyDeal_D5_V0 = Skulls_Mugging_D5_V0
Skulls_ShadyDeal_D5_V1 = Skulls_Mugging_D5_V0
Skulls_ShadyDeal_D5_V2 = Skulls_Mugging_D5_V0

Skulls_Mugging_D10_V0 = {
    ["Markers"] = {
            ["Encounter_V_40"] = Skulls_Ranks_01.Victims,
            ["Encounter_V_42"] = Skulls_Ranks_01.Victims,
            ["Encounter_S_30"] = Skulls_Ranks_01.Minions,
            ["Encounter_S_32"] = Skulls_Ranks_01.Boss,
            ["Encounter_E_03"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_04"] = Skulls_Ranks_01.Lieutenants,
            ["Encounter_E_07"] = Skulls_Ranks_01.Lieutenants,
            ["Encounter_E_01"] = Skulls_Ranks_01.Minions,           
   },
}

Skulls_Mugging_D10_V1 = Skulls_Mugging_D10_V0
Skulls_Mugging_D10_V2 = Skulls_Mugging_D10_V0
Skulls_Mugging_D10_V3 = Skulls_Mugging_D10_V0
Skulls_ShadyDeal_D10_V0 = Skulls_Mugging_D10_V0
Skulls_ShadyDeal_D10_V1 = Skulls_Mugging_D10_V0
Skulls_ShadyDeal_D10_V2 = Skulls_Mugging_D10_V0
SkullsTrolls_Deal_D10_V0 = Skulls_Mugging_D10_V0
SkullsTrolls_Deal_D10_V1 = Skulls_Mugging_D10_V0
SkullsTrolls_Deal_D10_V2 = Skulls_Mugging_D10_V0

--== PURSE SNATCHINGS ==--

Skulls_PurseSnatch_D1_V0 = {
    ["Markers"] = {
            ["Encounter_V_40"] = Skulls_Ranks_01.FemaleVictims,
            ["Encounter_S_30"] = Skulls_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Skulls_Ranks_01.Minions,
   },
}

Skulls_PurseSnatch_D1_V1 = {
    ["Markers"] = {
            ["Encounter_V_40"] = Skulls_Ranks_01.FemaleVictims,
            ["Encounter_S_30"] = Skulls_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_03"] = Skulls_Ranks_01.Lieutenants,
   },
}

Skulls_PurseSnatch_D1_V2 = {
    ["Markers"] = {
            ["Encounter_V_40"] = Skulls_Ranks_01.FemaleVictims,
            ["Encounter_V_41"] = Skulls_Ranks_01.Victims,
            ["Encounter_S_30"] = Skulls_Ranks_01.Lieutenants,
            ["Encounter_S_31"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_03"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_05"] = Skulls_Ranks_01.Lieutenants,
   },
}

Skulls_PurseSnatch_D5_V0 = Skulls_PurseSnatch_D1_V0
Skulls_PurseSnatch_D5_V1 = Skulls_PurseSnatch_D1_V1
Skulls_PurseSnatch_D5_V2 = Skulls_PurseSnatch_D1_V2

Skulls_PurseSnatch_D10_V0 = Skulls_PurseSnatch_D1_V0
Skulls_PurseSnatch_D10_V1 = Skulls_PurseSnatch_D1_V1
Skulls_PurseSnatch_D10_V2 = Skulls_PurseSnatch_D1_V2

--== VANDALISM ==--

Skulls_Vandalism_D1_V0 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Skulls_Ranks_01.Minion,
            ["Encounter_E_01"] = Skulls_Ranks_01.Lieutenants,
   },
}

Skulls_Vandalism_D1_V1 = Skulls_Vandalism_D1_V0

Skulls_Vandalism_D5_V0 = {
    ["Markers"] = {
            ["Encounter_S_31"] = Skulls_Ranks_01.Minion,
            ["Encounter_S_30"] = Skulls_Ranks_01.Lieutenants,
            ["Encounter_E_01"] = Skulls_Ranks_01.Minion,
            ["Encounter_E_06"] = Skulls_Ranks_01.Minion,
   },
}

Skulls_Vandalism_D5_V1 = Skulls_Vandalism_D5_V0

Skulls_Vandalism_D10_V0 = {
    ["Markers"] = {
            ["Encounter_S_31"] = Skulls_Ranks_01.Minion,
            ["Encounter_S_30"] = Skulls_Ranks_01.Lieutenants,
            ["Encounter_E_01"] = Skulls_Ranks_01.Minion,
            ["Encounter_E_06"] = Skulls_Ranks_01.Minion,
            ["Encounter_E_03"] = Skulls_Ranks_01.Minion,
            ["Encounter_E_07"] = Skulls_Ranks_01.Lieutenants,                        
   },
}

Skulls_Vandalism_D10_V1 = Skulls_Vandalism_D10_V0

--== BREAK-INS ==--

Skulls_BreakIn_D1_V0 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Skulls_Ranks_01.Minion,
            ["Encounter_E_01"] = Skulls_Ranks_01.Lieutenants,
   },
}

Skulls_BreakIn_D1_V1 = {
    ["Markers"] = {
            ["Encounter_S_31"] = Skulls_Ranks_01.Minion,
            ["Encounter_E_01"] = Skulls_Ranks_01.Minion,
            ["Encounter_E_06"] = Skulls_Ranks_01.Minion,
   },
}

Skulls_BreakIn_D1_V2 = {
    ["Markers"] = {
            ["Encounter_S_31"] = Skulls_Ranks_01.Minion,
            ["Encounter_S_30"] = Skulls_Ranks_01.Lieutenants,
            ["Encounter_E_07"] = Skulls_Ranks_01.Lieutenants,                        
   },
}

Skulls_BreakIn_D5_V0 = Skulls_BreakIn_D1_V0
Skulls_BreakIn_D5_V1 = Skulls_BreakIn_D1_V1
Skulls_BreakIn_D5_V2 = Skulls_BreakIn_D1_V2

Skulls_BreakIn_D10_V0 = Skulls_BreakIn_D1_V0
Skulls_BreakIn_D10_V1 = Skulls_BreakIn_D1_V1
Skulls_BreakIn_D10_V2 = Skulls_BreakIn_D1_V2

--==FIGHT CLUB==--

Skulls_FightClub_D3_V0 = {
    ["Markers"] = {
            ["Encounter_S_32"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_03"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_07"] = Skulls_Ranks_01.Minions,
   },
}

Skulls_FightClub_D3_V1 = Skulls_FightClub_D3_V0
Skulls_FightClub_D3_V2 = Skulls_FightClub_D3_V0

Skulls_FightClub_D8_V0 = {
    ["Markers"] = {
            ["Encounter_S_32"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_03"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_07"] = Skulls_Ranks_01.Minions,
   },
}

Skulls_FightClub_D8_V1 = Skulls_FightClub_D8_V0
Skulls_FightClub_D8_V2 = Skulls_FightClub_D8_V0

--== BEAT DOWN ==--

Skulls_BeatDown_D3_V0 = {
    ["Markers"] = {
            ["Encounter_S_32"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_03"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_07"] = Skulls_Ranks_01.Minions,
   },
}

Skulls_BeatDown_D3_V1 = Skulls_BeatDown_D3_V0
Skulls_BeatDown_D3_V2 = Skulls_BeatDown_D3_V0

Skulls_BeatDown_D8_V0 = {
    ["Markers"] = {
            ["Encounter_S_32"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_03"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_07"] = Skulls_Ranks_01.Minions,
   },
}

Skulls_BeatDown_D8_V1 = Skulls_BeatDown_D8_V0
Skulls_BeatDown_D8_V2 = Skulls_BeatDown_D8_V0

--==RUMBLE: Skull vs. Vahzilok==--

SkullsVahzilok_Rumble_D1_V0 = {
    ["Markers"] = {
        ["Encounter_E_09"] = Skulls_Ranks_01.Minions,
        ["Encounter_E_03"] = Skulls_Ranks_01.Lieutenants,
        ["Encounter_E_01"] = Skulls_Ranks_01.Minions,
        ["Encounter_E_05"] = Skulls_Ranks_01.Minions,
        ["Encounter_S_33"] = Skulls_Ranks_01.Minions,
        ["Encounter_S_31"] = Skulls_Ranks_01.Minions,
        ["Encounter_E_02"] = Vahzilok_Rumble_01.Minions,
        ["Encounter_E_10"] = Vahzilok_Rumble_01.Minions,
        ["Encounter_E_04"] = Vahzilok_Rumble_01.Minions,
        ["Encounter_E_06"] = Vahzilok_Rumble_01.Lieutenants,
        ["Encounter_S_32"] = Vahzilok_Rumble_01.Minions,
        ["Encounter_S_30"] = Vahzilok_Rumble_01.Minions,           
   },
}

SkullsVahzilok_Rumble_D1_V1 = SkullsVahzilok_Rumble_D1_V0
SkullsVahzilok_Rumble_D1_V2 = SkullsVahzilok_Rumble_D1_V0
SkullsVahzilok_Rumble_D1_V3 = SkullsVahzilok_Rumble_D1_V0

SkullsVahzilok_Rumble_D5_V0 = {
    ["Markers"] = {
        ["Encounter_E_09"] = Skulls_Ranks_01.Minions,
        ["Encounter_E_03"] = Skulls_Ranks_01.Lieutenants,
        ["Encounter_E_01"] = Skulls_Ranks_01.Minions,
        ["Encounter_S_33"] = Skulls_Ranks_01.Minions,
        ["Encounter_S_31"] = Skulls_Ranks_01.Minions,
        ["Encounter_S_35"] = Skulls_Ranks_01.Boss,
        ["Encounter_E_10"] = Vahzilok_Rumble_01.Minions,
        ["Encounter_E_12"] = Vahzilok_Rumble_01.Lieutenants,
        ["Encounter_E_08"] = Vahzilok_Rumble_01.Boss,
        ["Encounter_S_32"] = Vahzilok_Rumble_01.Minions,
        ["Encounter_S_34"] = Vahzilok_Rumble_01.Lieutenants,
        ["Encounter_S_30"] = Vahzilok_Rumble_01.Minions,           
   },
}

SkullsVahzilok_Rumble_D5_V1 = SkullsVahzilok_Rumble_D5_V0
SkullsVahzilok_Rumble_D5_V2 = SkullsVahzilok_Rumble_D5_V0
SkullsVahzilok_Rumble_D5_V3 = SkullsVahzilok_Rumble_D5_V0

SkullsVahzilok_Rumble_D10_V0 = {
    ["Markers"] = {
            ["Encounter_E_09"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_03"] = Skulls_Ranks_01.Lieutenants,
            ["Encounter_E_01"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_11"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_05"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_07"] = Skulls_Ranks_01.Lieutenants,
            ["Encounter_S_33"] = Skulls_Ranks_01.Minions,
            ["Encounter_S_31"] = Skulls_Ranks_01.Minions,
            ["Encounter_S_35"] = Skulls_Ranks_01.Minions,
            ["Encounter_E_02"] = Vahzilok_Rumble_01.Minions,
            ["Encounter_E_10"] = Vahzilok_Rumble_01.Minions,
            ["Encounter_E_04"] = Vahzilok_Rumble_01.Minions,
            ["Encounter_E_12"] = Vahzilok_Rumble_01.Lieutenants,
            ["Encounter_E_06"] = Vahzilok_Rumble_01.Lieutenants,
            ["Encounter_E_08"] = Vahzilok_Rumble_01.Minions,
            ["Encounter_S_32"] = Vahzilok_Rumble_01.Minions,
            ["Encounter_S_34"] = Vahzilok_Rumble_01.Minions,
            ["Encounter_S_30"] = Vahzilok_Rumble_01.Minions,
   },
}

SkullsVahzilok_Rumble_D10_V1 = SkullsVahzilok_Rumble_D10_V0
SkullsVahzilok_Rumble_D10_V2 = SkullsVahzilok_Rumble_D10_V0
SkullsVahzilok_Rumble_D10_V3 = SkullsVahzilok_Rumble_D10_V0