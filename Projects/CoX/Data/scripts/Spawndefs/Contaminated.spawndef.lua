--This is a rank table
--There could be multiple tables to generate spawns from


local Contaminated_Ranks_01  = {
        ["Underlings"] = {
          --NA
        },
        ["Minions"] = {
                "Thug_Contaminated_01", "Thug_Contaminated_02", "Thug_Contaminated_03",
                "Thug_Contaminated_04", "Thug_Contaminated_05", "Thug_Contaminated_06",
                "Thug_Contaminated_07", "Thug_Contaminated_08", "Thug_Contaminated_09",
                "Thug_Contaminated_10",
        },
        ["Lieutenants"] = {
          --NA
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

        },
        ["Specials"] = {

        },
}

--[[
        These are spawndef variantions.
]]

Contaminated_D1_V0 = {
        ["Markers"] = {
                ["Encounter_S_32"] = Contaminated_Ranks_01.Minions,
                ["Encounter_E_05"] = Contaminated_Ranks_01.Minions,
        },
}