--These contain tables for relevant, spawnable NPCs
include_lua('./Spawndefs/Contaminated_Outbreak.spawndef.lua')
include_lua('./Spawndefs/Police_Robots_Outbreak.spawndef.lua')
include_lua('./Spawndefs/Shady_Deal_Cop_Outbreak.spawndef.lua')

--[[
  NOTE:  Neither the city cops or Robots (with no #) are used for some reason.
  Robots1 is the downed drone + tech group (grp2232)
  Robots2 and Robots3 are the training rikti drones

  NOTE: Each entry can have multiple CanSpawns but they must be
  sequential. There must always be a CanSpawn1 at minimum.
  Adding more will (later) increase the number of squads spawned.
]]

ES_Library_Objects = {
  ["ES_Contaminated_City_00_01"] = {
    ["EncounterSpawn"] = "CenteredAround",
    ["CanSpawn"] = {"CanSpawn1"},
    ["CanSpawnDefs"] = {Contaminated_D1_V0},
  },
  ["ES_Cops_City_00_01"] = {
    ["EncounterSpawn"] = "CenteredAround",
    ["CanSpawn"] = {"CanSpawn1",},
    ["CanSpawnDefs"] = {Shady_Deal_Cops_D1_V0,},
  },
  ["ES_Robots_City_00_01"] = {
    ["EncounterSpawn"] = "Phalanx",
    ["CanSpawn"] = {"CanSpawn1",},
    ["CanSpawnDefs"] = {Robot1_D1_V0,},
  },
  ["ES_Robot1_City_00_01"] = {
    "Encounter_E_03", "Encounter_E_04",
    ["EncounterSpawn"] = "CenteredAround",
    ["SpawnProbability"] = 100,    
    ["CanSpawn"] = {"CanSpawn1",},
    ["CanSpawnDefs"] = {Robot1_D1_V0,},
  },
  ["ES_Robot2_City_00_01"] = {
    ["EncounterSpawn"] = "Encounter",
    ["SpawnProbability"] = 100,        
    ["CanSpawn"] = {"CanSpawn1",},
    ["CanSpawnDefs"] = {Robot2_D10_V0,},
  },
  ["ES_Robot3_City_00_01"] = {
    ["EncounterSpawn"] = "Encounter",
    ["SpawnProbability"] = 100,        
    ["CanSpawn"] = {"CanSpawn1",},
    ["CanSpawnDefs"] = {Robot3_D5_V0,},
  },
}

--REQUIRED: This inserts universal into this map's OL
local UniOnce
if UniOnce == false or UniOnce == nil then
    InsertUniversals()
    UniOnce = true
end