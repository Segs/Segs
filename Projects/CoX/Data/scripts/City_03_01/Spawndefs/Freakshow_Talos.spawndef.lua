--This is a rank table
--There could be multiple tables to generate spawns from
local Freakshow_Ranks_01  = {
    ["Underlings"] = {
      --NA
    },
    ["Minions"] = {
        "FRK_01","FRK_02","FRK_03",
        "FRK_04","FRK_05","FRK_06",
        "FRK_07","FRK_08","FRK_09",
        "FRK_10","FRK_11","FRK_12",
        "FRK_13","FRK_14","FRK_15",
        "FRK_16","FRK_17","FRK_21",
        "FRK_22","FRK_23","FRK_24",
        "FRK_25","FRK_26","FRK_27",
        "FRK_28",
    },
    ["Lieutenants"] = {
        "FRK_31","FRK_32","FRK_33",
        "FRK_34","FRK_35","FRK_36",
        "FRK_37","FRK_38","FRK_50",
        "FRK_51","FRK_52","FRK_53",
        "FRK_54",
    },
    ["Sniper"] = {
      --NA
    },
    ["Boss"] = {
        "FRK_41","FRK_42","FRK_43",
        "FRK_44","FRK_45","FRK_46",
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
    ["Specials"] = {
        -- Missing the MeatDoctor
    },
}

Vandalism_Freakshow_D1_V0 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Freakshow_Ranks_01.Minions,
        ["Encounter_E_04"] = Freakshow_Ranks_01.Boss,            
    },
}

Vandalism_Freakshow_D1_V1 = {
    ["Markers"] = {
        ["Encounter_S_30"] = Freakshow_Ranks_01.Minions,
        ["Encounter_E_02"] = Freakshow_Ranks_01.Minions,
        ["Encounter_E_05"] = Freakshow_Ranks_01.Boss,
        ["Encounter_E_06"] = Freakshow_Ranks_01.Minions,            
    },
}

Vandalism_Freakshow_D1_V2 = Vandalism_Freakshow_D1_V0
Vandalism_Freakshow_D1_V3 = Vandalism_Freakshow_D1_V1

Vandalism_Freakshow_D5_V0 = Vandalism_Freakshow_D1_V0
Vandalism_Freakshow_D5_V1 = Vandalism_Freakshow_D1_V1
Vandalism_Freakshow_D5_V2 = Vandalism_Freakshow_D1_V0
Vandalism_Freakshow_D5_V3 = Vandalism_Freakshow_D1_V1

Vandalism_Freakshow_D10_V0 = Vandalism_Freakshow_D1_V0
Vandalism_Freakshow_D10_V1 = Vandalism_Freakshow_D1_V1
Vandalism_Freakshow_D10_V2 = Vandalism_Freakshow_D1_V0
Vandalism_Freakshow_D10_V3 = Vandalism_Freakshow_D1_V1

Harrass_Freakshow_D1_V0 = {
    ["Markers"] = {
        ["Encounter_E_01"] = Freakshow_Ranks_01.Minions,
        ["Encounter_E_02"] = Freakshow_Ranks_01.Minions,
        ["Encounter_E_06"] = Freakshow_Ranks_01.Minions,
        ["Encounter_S_30"] = Freakshow_Ranks_01.Lieutenants,
    },
}

Harrass_Freakshow_D1_V1 = Harrass_Freakshow_D1_V0
Harrass_Freakshow_D1_V2 = Harrass_Freakshow_D1_V0
Harrass_Freakshow_D1_V3 = Harrass_Freakshow_D1_V0

Harrass_Freakshow_D5_V0 = Harrass_Freakshow_D1_V0
Harrass_Freakshow_D5_V1 = Harrass_Freakshow_D1_V0
Harrass_Freakshow_D5_V2 = Harrass_Freakshow_D1_V0
Harrass_Freakshow_D5_V3 = Harrass_Freakshow_D1_V0

Harrass_Freakshow_D10_V0 = Harrass_Freakshow_D1_V0
Harrass_Freakshow_D10_V1 = Harrass_Freakshow_D1_V0
Harrass_Freakshow_D10_V2 = Harrass_Freakshow_D1_V0
Harrass_Freakshow_D10_V3 = Harrass_Freakshow_D1_V0
