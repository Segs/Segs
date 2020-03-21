--This script is primarily used to load scenegraph data into EMs and handle mass spawning/despawning events

MapEncPool = {}         -- used in place of a currently non-existent zone manager

--This both locates and spawns Persistent NPCs in the scenegraph
function spinPersists()
    totalCount = MapInstance.GetPersistentCount()

    --print("-----------------------------------------")
    --print("Persistent count is " .. tostring(totalCount))

    if totalCount == 0 or totalCount == nil then
        print("No persistents located or peristents invalid.")
        return ""
    end

    for i = 1, totalCount do
        PersName = MapInstance.GetPersistentName(i-1)
        --filters out the "0" node persistent spawns.
        --Should handle this during the scenegraph parse if they prove extraneous
        if PersName == 0 or PersName == nil or PersName == "0" then
            --print("PERS ZERO: " .. tostring(PersName) .. " at " .. round(PersPos.x) .. " " .. round(PersPos.y) .. " " .. round(PersPos.z))
            goto done
        end
        PersModel = GetPersistentModel(PersName)
        PersPos = MapInstance.GetPersistentPosition(i-1)
        PersRot = MapInstance.GetPersistentRotation(i-1)
        local newID = spawnCivilian(PersModel, PersPos, PersRot)
        --print("Persistent ID: " .. tostring(newID))
        --print("PERS: " .. tostring(PersName) .. " at " .. round(PersPos.x) .. " " .. round(PersPos.y) .. " " .. round(PersPos.z))
        ::done::
    end  

end

--locates NPC spawn/generate nodes available
--For now, it also spawns them, but some sort of manager should do so in the future
function spinCivilians()
    totalCount = MapInstance.GetCivCount()
    --print("-----------------------------------------")
    --print("Civ Spawner count is " .. tostring(totalCount))

    if totalCount == 0 or totalCount == nil then
        print("No civ spawners located or civ spawners invalid.")
        return ""       -- nothing to do!
    end
end

--This locates NPC spawn/generate nodes available
--For now, it also spawns them, but some sort of manager should do so in the future
function spinCars()
    totalCount = MapInstance.GetCarCount()
    --print("-----------------------------------------")
    --print("Car Spawner count is " .. tostring(totalCount))

    if totalCount == 0 or totalCount == nil then
        print("No car spawners located or car spawners invalid.")
        return ""       -- nothing to do!
    end    
end

function spinSpawners()
    totalCount = MapInstance.GetSpawnerCount()
    --print("-----------------------------------------")
    --print("Spawner count is " .. tostring(totalCount))

    if totalCount == 0 or totalCount == nil then
        print("No spawners located or spawners invalid.")
        return ""       -- nothing to do!
    end

    for i = 1, totalCount do
        CreateEncounterManager(MapEncPool, MapInstance.GetSpawnerPosition(i-1))
        local EncName = MapInstance.GetSpawnerName(i-1)
        MapEncPool[i].EncounterName = EncName
        MapEncPool[i].Markers = {}
        MapEncPool[i].MyCritters = {}           --must initialize this here
        MapEncPool[i].PanicInitiators = {}      --must initialize this here

        local validateSpawnDef = IsSpawnDef(EncName)
        if validateSpawnDef ~= nil and validateSpawnDef == true then        
            MapEncPool[i].SpawnProbabilityBase = GetLOSpawnProbability(EncName)
        end

        local MarkerCount = MapInstance.GetSpawnerChildCount(i-1)
        
        for m = 1, MarkerCount do
            local ChildName = MapInstance.GetSpawnerChildName(i-1, m-1)
            local ChildPos = MapInstance.GetSpawnerChildPosition(i-1, m-1)
            local ChildRot = MapInstance.GetSpawnerChildRotation(i-1, m-1)

            --print(ChildName .. " " .. ChildPos.x .. " " .. ChildPos.y .. " " .. ChildPos.z)
            table.insert(MapEncPool[i].Markers, m, Marker:new())
            MapEncPool[i].Markers[m].MarkerName = ChildName
            MapEncPool[i].Markers[m].Position = ChildPos
            MapEncPool[i].Markers[m].Rotation = ChildRot
        end
    end
end 

--simple rounding function
function round(x)
    return x>=0 and math.floor(x+0.5) or math.ceil(x-0.5)
end

--This cycles through encounter spawners available and spawns
--any encounters with 100% spawn probability
--Note that the way probability works (I believe) is that
--an encounter will respawn based on that probability, once defeated
--So a 100% SP encounter will always return, an 80% returns 4/5 times, etc.
function InitiateKillPersists()
    local Count = #MapEncPool
    for i = 1, Count do
        local Base, Adj = MapEncPool[i]:GetSpawnProbability()
        local curState = MapEncPool[i]:GetState()

        if (Base + Adj) >= 100 
        and (curState == "STATUS_RUNNING_IDLE" or curState == "STATUS_SLEEPING") then
            MapEncPool[i]:Spawn()
        end
    end
end

-- Total is the # of attempts made to spawn, not necessarily the actual number
-- This also calls 100% probability spawns to generate if they haven't already
-- Those do not count against the requested total & complete with Encounter/Spawndef types only
function RandomSpawn(total, type)
    math.randomseed(DateTime.SecsSince2000Epoch())
    InitiateKillPersists()         --ensure 100% probability spawns occur    
    local spawned = 0

    if total == nil or total == 0 then
        total = 1
    end
    
    if type == nil or type == "Encounter" then
        for i = 1, total do
            local count = #MapEncPool
            if count == 0 or count == nil then
                print("Spawner pool doesn't exist. Exiting.")
                return ""
            end

            local currentEnc = math.random(1, count)
            local curState = MapEncPool[currentEnc]:GetState()
            if curState == "STATUS_RUNNING_IDLE" or curState == "STATUS_SLEEPING" then
                MapEncPool[currentEnc]:Spawn()
                spawned = spawned + 1
            end
        end
        --print("Random Spawn generated " .. spawned .. " encounters.")        
    
    elseif type == "Civilians" then
        spawned = 0
        --Note: This may currently cause more than one civilian to spawn at the same location
        for i = 1, total do
            local count = MapInstance.GetCivCount()
            if count == 0 or count == nil then
                print("Civ spawner pool doesn't exist. Exiting.")
                return ""
            end

            local CurrentCiv = math.random(1, count)
            local CivToSpawn = math.random(1, #All_CoH_Civilians)
            local CurRot = MapInstance.GetCivRotation(CurrentCiv-1)
            

            local CurPos = MapInstance.GetCivPosition(CurrentCiv-1)
            --slight adjustment way from pole
            CurPos.x = CurPos.x - math.random(6.0, 8.0)
            CurPos.z = CurPos.z - math.random(2.0, 10.0)

            --apply random rotation before spawning
            CurRot.y = CurRot.y - math.random(-2, 2)
            spawnCivilian(All_CoH_Civilians[CivToSpawn], CurPos, CurRot)
            spawned = spawned + 1
        end

        --print("Random Spawn generated " .. spawned .. " civilians.") 
    elseif type == "Cars" then
        spawned = 0
        --Note: This may currently cause more than one civilian to spawn at the same location
        for i = 1, total do
            local count = MapInstance.GetCarCount()
            if count == 0 or count == nil then
                print("Car spawner pool doesn't exist. Exiting.")
                return ""
            end

            local CurrentCiv = math.random(1, count)
            local CurPos = MapInstance.GetCarPosition(CurrentCiv-1)
            --slight adjustment way from pole
            CurPos.x = CurPos.x - math.random(2.0, 8.0)
            local CurRot = MapInstance.GetCarRotation(CurrentCiv-1)
            spawned = spawned + 1
            local CarToSpawn = math.random(1, #All_Cars)
            spawnCivilian(All_Cars[CarToSpawn], CurPos, CurRot)
        end        
        --print("Car Spawn generated " .. spawned .. " cars.") 
    end

    return spawned
end

--Use "override" to also despawn 100% spawn probability critters
--This could be streamlined some to issue fewer despawn calls,
--but for now it ensures the map is truly clean
function DespawnMapEncounters(override)

    for i = 1, #MapEncPool do
        MapEncPool[i]:Despawn(override)
    end

end
