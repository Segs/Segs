--This is a rank table
--There could be multiple tables to generate spawns from
local Outcasts_Ranks_01 = {
    ["Underlings"] = {
      --NA
    },
    ["Minions"] = {
        "Thug_Outcast_01","Thug_Outcast_02","Thug_Outcast_03",
        "Thug_Outcast_04","Thug_Outcast_05","Thug_Outcast_06",
        "Thug_Outcast_07","Thug_Outcast_08",
    },
    ["Lieutenants"] = {
    },
    ["Sniper"] = {
      --NA
    },
    ["Boss"] = {
        "Thug_Outcast_Boss_01","Thug_Outcast_Boss_02","Thug_Outcast_Boss_03",
        "Thug_Outcast_Boss_04","Thug_Outcast_Boss_05","Thug_Outcast_Boss_06",
        "Thug_Outcast_Boss_07","Thug_Outcast_Boss_08",
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
    ["Specials"] = { -- Are these sepecials, normal or something else?
        "DeadThug_Outcast_01","DeadThug_Outcast_02","DeadThug_Outcast_03",
        "DeadThug_Outcast_04","DeadThug_Outcast_05","DeadThug_Outcast_06",
        "DeadThug_Outcast_07","DeadThug_Outcast_08",

        "DeadThug_Outcast_Boss_01","DeadThug_Outcast_Boss_02","DeadThug_Outcast_Boss_03",
        "DeadThug_Outcast_Boss_04","DeadThug_Outcast_Boss_05","DeadThug_Outcast_Boss_06",
        "DeadThug_Outcast_Boss_07","DeadThug_Outcast_Boss_08",
    },
}

local Trolls_Rumble_01 = {
      ["Minions"] = {
          "Thug_Troll_01","Thug_Troll_02","Thug_Troll_03",
          "Thug_Troll_04","Thug_Troll_05","Thug_Troll_06",
      },
      ["Boss"] = {
          "Thug_Troll_Boss_01", "Thug_Troll_Boss_02", "Thug_Troll_Boss_03"
      },
}

--== LOITER ==--

Loiter_Outcasts_L11_13_V0 = {
    ["Markers"] = {
        ["Encounter_S_32"] = Outcasts_Ranks_01.Boss,
        ["Encounter_S_30"] = Outcasts_Ranks_01.Boss,
        ["Encounter_E_03"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_07"] = Outcasts_Ranks_01.Minions,            
    },
}

Loiter_Outcasts_L14_17_V0 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
            ["Encounter_S_32"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_01"] = Outcasts_Ranks_01.Boss,
            ["Encounter_E_04"] = Outcasts_Ranks_01.Boss,
    },
}

Loiter_Outcasts_L18_20_V0 = {
    ["Markers"] = {
            ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
            ["Encounter_S_32"] = Outcasts_Ranks_01.Minions,
            ["Encounter_E_01"] = Outcasts_Ranks_01.Boss,
            ["Encounter_E_04"] = Outcasts_Ranks_01.Boss,
    },
}

Loiter_Outcasts_L11_13_V0 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_32"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Boss,
        ["Encounter_E_06"] = Outcasts_Ranks_01.Boss,
        ["Encounter_E_07"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_08"] = Outcasts_Ranks_01.Minions,
    },
}

--== FIGHT CLUB ==--

FightClub_Outcasts_L12_15_V0 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_32"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_01"] = Outcasts_Ranks_01.Boss,
        ["Encounter_E_02"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_03"] = Outcasts_Ranks_01.Boss,
        ["Encounter_E_04"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_08"] = Outcasts_Ranks_01.Boss,
    },
}

FightClub_Outcasts_L16_19_V0 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_32"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_01"] = Outcasts_Ranks_01.Boss,
        ["Encounter_E_02"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_03"] = Outcasts_Ranks_01.Boss,
        ["Encounter_E_04"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_08"] = Outcasts_Ranks_01.Boss,
    },
}

--== RUMBLE ==--

Rumble_OutcastsTrolls_L11_13_V0 = {
    ["Markers"] = {
        ["Encounter_E_09"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_03"] = Outcasts_Ranks_01.Boss,
        ["Encounter_E_01"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_05"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_33"] = Outcasts_Ranks_01.Minions,
        ["Encounter_S_31"] = Outcasts_Ranks_01.Minions,
        ["Encounter_E_02"] = Trolls_Rumble_01.Minions,
        ["Encounter_E_10"] = Trolls_Rumble_01.Minions,
        ["Encounter_E_04"] = Trolls_Rumble_01.Minions,
        ["Encounter_E_06"] = Trolls_Rumble_01.Boss,
        ["Encounter_S_32"] = Trolls_Rumble_01.Minions,
        ["Encounter_S_30"] = Trolls_Rumble_01.Minions,           
   },
}

Rumble_OutcastsTrolls_L14_17_V0 = Rumble_OutcastsTrolls_L11_13_V0
Rumble_OutcastsTrolls_L18_20_V0 = Rumble_OutcastsTrolls_L11_13_V0