--This script is used to list universal entries
--appending them to ES_Library_Objects that appears for each map
--It must therefore be loaded after the map's ES_Library_Objects.lua

--Universal spawndef for police drones
UNI_PoliceDroneGuards_V01 = {
    ["Markers"] = {
            ["Encounter_E_01"] = "Police_Drone_Guard",
    },
}

UNI_Consult_V01 = {
    ["Markers"] = {
            ["Encounter_E_01"] = {"MaleNPC_200","MaleNPC_201","MaleNPC_202","MaleNPC_203",},
            ["Encounter_E_02"] = {"FemaleNPC_200","FemaleNPC_201","FemaleNPC_101",},
    },
}

UNI_Consult_Pat_V01 = {
    ["Markers"] = {
            ["Encounter_E_01"] = "MaleNPC_80",
            ["Encounter_E_02"] = {"MaleNPC_200","MaleNPC_201","MaleNPC_202","MaleNPC_203",},
    },
}

--Just using a mix of generic npc models for now
UNI_Clipboard_V01 = {
    ["Markers"] = {
            ["Encounter_E_01"] = {
                "FemaleNPC_01",
                "MaleNPC_01",
            },
    },
}

--Just using a mix of generic npc models for now
UNI_Cell_Conv_V01 = {
    ["Markers"] = {
            ["Encounter_E_01"] = {
                "FemaleNPC_30",
                "FemaleNPC_31",
                "FemaleNPC_32",
                "FemaleNPC_33",
                "FemaleNPC_34",
                "MaleNPC_31",
                "MaleNPC_32",
                "MaleNPC_33",
                "MaleNPC_34",
            },
    },
}

--Not sure what model(s) are used for reporters yet; mixed bum/work/cop for now
UNI_Reporter_V01 = {
    ["Markers"] = {
            ["Encounter_E_01"] = {
                "MaleNPC_110",
                "MaleNPC_111",
                "MaleNPC_112",
                "MaleNPC_113",
                "MaleNPC_114",
                "MaleNPC_115",
                "MaleNPC_210",
                "MaleNPC_220",
                "MaleNPC_221",
            },
    }
}

--Library Object data to be inserted
Universal_OL = {
    ["_ES_PoliceDrone"] = {
        "Encounter_E_01",
        ["EncounterSpawn"] = "Single",
        ["SpawnProbability"] = 100,        
        ["CanSpawn"] = {"CanSpawn1",},
        ["CanSpawnDefs"] = {UNI_PoliceDroneGuards_V01,},
    },
    ["ES_Consult_Patient_NPC_City_01_03"] = {
        "Encounter_E_01", "Encounter_E_02",
        ["EncounterSpawn"] = "Double",
        ["SpawnProbability"] = 100,        
        ["CanSpawn"] = {"CanSpawn1",},
        ["CanSpawnDefs"] = {UNI_Consult_Pat_V01,},        
    },
    ["ES_Consult_NPC_City_01_03"] = {
        "Encounter_E_01", "Encounter_E_02",
        ["EncounterSpawn"] = "Double",
        ["SpawnProbability"] = 100,        
        ["CanSpawn"] = {"CanSpawn1",},
        ["CanSpawnDefs"] = {UNI_Consult_V01,},        
    },
    ["ES_Cellphone_NPC_City_01_03"] = {
        "Encounter_E_01",
        ["EncounterSpawn"] = "Single",
        ["SpawnProbability"] = 100,        
        ["CanSpawn"] = {"CanSpawn1",},
        ["CanSpawnDefs"] = {UNI_Cell_Conv_V01,},        
    },
    ["ES_Newspaper_NPC_City_01_03"] = {
        "Encounter_E_01",
        ["EncounterSpawn"] = "Single",
        ["SpawnProbability"] = 100,        
        ["CanSpawn"] = {"CanSpawn1",},
        ["CanSpawnDefs"] = {UNI_Reporter_V01,},        
    },
    ["ES_Clipboard_NPC_City_01_03"] = {
        "Encounter_E_01",
        ["EncounterSpawn"] = "Single",
        ["SpawnProbability"] = 100,        
        ["CanSpawn"] = {"CanSpawn1",},
        ["CanSpawnDefs"] = {UNI_Clipboard_V01,},        
    },                      
}

--Injects universal spawndefs into a map's ES_Library_Objects
function InsertUniversals()
    for k,v in pairs(Universal_OL) do
        --print("K: " .. tostring(k) .. " V: " .. tostring(v))
        ES_Library_Objects[k] = v
    end
end