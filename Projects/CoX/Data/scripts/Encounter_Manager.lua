--Sends a server message; used for manual/human reporting
function CallOut(message)
        MapClientSession.SendInfoMessage(13, tostring(message))
        return ""
end

--Prototype
--TODO: Polling Routine
--TODO: Function to query EMs within minimum distance value
--TODO: Function to determine distance to nearest player

--Status "cosntants"
local STATUS_SLEEPING = 1
local STATUS_RUNNING_IDLE = 2
local STATUS_RUNNING_ACTIVE = 3
local STATUS_RUNNING_PANIC = 4
local STATUS_RUNNING_DESPAWN = 5
local STATUS_INVALID = 1000

--Several of these values should be drawn from a config file.
EncounterManager = {
        ["MyCritters"] = {},                    --list of spawned critters; initiatlize at EM creation
        ["Position"] = vec3.new(0, 0, 0),
        ["StatusCurrent"] = STATUS_RUNNING_IDLE,
        ["StatusPrevious"] = STATUS_RUNNING_IDLE,
        ["EncounterIsActive"] = false,
        ["PollingIntervalBase"] = 60.0,
        ["PollingIntervalAdj"] = 0,
        ["PanicDuration"] = 90.0,
        ["PanicInitiators"] = {},               --player or team|players inciting panic (encounter aggrolist)
        ["SpawnProbabilityBase"] = 80,
        ["SpawnProbabilityAdj"] = 0,
        ["EncounterName"] = "",                 --encounter spawner name; used for OL lookups
        ["Markers"] = {}                        --spawnable markers for this encounter
}

function EncounterManager:new (o)
    o = o or {}   --new object if needed
    setmetatable(o, self)
    self.__index = self
    return o
end

--Markers designate definite spawnable locations in the world
Marker = {
        ["MarkerName"] = "",
        ["Position"] = vec3.new(0, 0, 0),
        ["Rotation"] = vec3.new(0, 0, 0),
}

function Marker:new (o)
        o = o or {}
        setmetatable(o, self)
        self.__index = self
        return o
end
--Returns the # of critters in this EM
--[[
        NOTE: This is not working as critters aren't actually stored yet.
        We need any AddNpc styled functions to return an EntityIDX on creation,
        and then store them in Critters{} so we have a usable reference for despawning
        and other activities.
]]
EncounterManager.CountCritters = function (self)
        local Counter = #self.MyCritters or 0
        return Counter
end

EncounterManager.AddCritterID = function (self, critterID)
        table.insert(self.MyCritters, critterID)
        return self:CountCritters()
end

--Handles state changes for the EM
EncounterManager.SetState = function (self, status)

        --TODO: Validate that the supplied status is valid

        self.StatusPrevious = self.StatusCurrent
        self.StatusCurrent = status

        if status == STATUS_SLEEPING then
              self.EncounterIsActive = false
        elseif status == STATUS_RUNNING_IDLE then
              self.EncounterIsActive = false
        elseif status == STATUS_RUNNING_ACTIVE then
              self.EncounterIsActive = true
        elseif status == STATUS_RUNNING_PANIC then
              --Start the panic timer
              --Panic Initiators need to be handled somewhere but likely not here.
              --Probably withhin whatever called this status change
        elseif status == STATUS_RUNNING_DESPAWN then
              --Indicates despawn has begun
              --Nothing else required at the moment
        end
end

--Returns a translated status message and allows for further handling if desired.
EncounterManager.GetState = function (self)

        local status = self.StatusCurrent

        if status == STATUS_SLEEPING then
                return "STATUS_SLEEPING"
        elseif status == STATUS_RUNNING_IDLE then
                return "STATUS_RUNNING_IDLE"
        elseif status == STATUS_RUNNING_ACTIVE then
                return "STATUS_RUNNING_ACTIVE"
        elseif status == STATUS_RUNNING_PANIC then
                return "STATUS_RUNNING_PANIC"
        elseif status == STATUS_RUNNING_DESPAWN then
                return "STATUS_RUNNING_DESPAWN"
        end

        return "STATUS_INVALID" --Something is wrong if we end up here.
end

-- returns both Spawn Probability values: base and adjustment
EncounterManager.GetSpawnProbability = function (self)
        local Base = self.SpawnProbabilityBase
        local Adj = self.SpawnProbabilityAdj
        if Base < 0 then Base = 0 end   -- Base shouldn't be below 0 or above 100
        if Base > 100 then Base = 100 end 

        return Base, Adj
end

--Used for manual human interactions
EncounterManager.ReportStatus = function (self)
        local Pos = self.Position
        local Alert = "I am at (" .. tostring(Pos.x) .. ", " .. tostring(Pos.y) .. ", " .. tostring(Pos.z) .. ")"
        Alert = Alert .. "\nMy status is: " .. self:GetState()
        CallOut(Alert)
        return ""
end

--Interacts with the Spawndef to create an active encounter
EncounterManager.Spawn = function (self)
        --Can't use os.time() for some reason? This should do?
        math.randomseed(DateTime.SecsSince2000Epoch() * self.Position.x)

        local Encounter = self.EncounterName

        --Handle nil and 0 probability encounters
        if Encounter == nil then
                print("Invalid encounter detected. Exiting.")
                return ""
        elseif (self.SpawnProbabilityBase + self.SpawnProbabilityAdj) <= 0 then
                return ""
        end

        local validateSpawnDef = IsSpawnDef(Encounter)
        if validateSpawnDef ~= nil and validateSpawnDef == true then
                local BaseType = GetBaseType(Encounter)
                --"Ambush" are for missions or special encounters only; exit.
                --"Encounter" types are for monsters and most/all(?) drones;
                if BaseType == nil or BaseType == "Ambush" then
                        print(tostring(Encounter) .. " Ambush, Special or NIL detected. Exiting.")
                        return ""  
                end
                --For now, get a random SpawnDef from those available
                local SquadCount = math.random(1, GetCanSpawnCount(Encounter))
                local Variant = {}
                Variant = GetVariant(Encounter, SquadCount)
                --Step through the template's markers and critters
                local Counter = 0
                for Tmarker, Tcritter in pairs(Variant["Markers"]) do
                        Counter = Counter + 1
                        --try to find a marker to match the Tmarker
                        local count = #self.Markers
                        local EncPos
                        local EncRot
                        
                        --this EM is improperly initiatlized or is broken, exit
                        if count == 0 or count == nil then
                                print(tostring(Encounter) .. " EM has no markers. Exiting.")
                                return ""
                        end

                        for i = 1, count do
                                --FOUND
                                if Tmarker == self.Markers[i].MarkerName then
                                        EncPos = self.Markers[i].Position
                                        EncRot = self.Markers[i].Rotation
                                end
                        end

                        --no marker found, or location data is bad.
                        if EncPos == nil then
                                print(tostring(Encounter) .. " EM has bad position data. Exiting.")
                                return ""                        
                        elseif EncRot == nil then
                                print(tostring(Encounter) .. " EM has bad rotation data. Exiting.")
                                return ""
                        end

                        local SpawnEntity
                        --if it's is a table, get a random critter instead
                        if type(Tcritter) == "table" then
                                SpawnEntity = Tcritter[math.random(1, #Tcritter)]
                        else
                                SpawnEntity = Tcritter
                        end

                        if SpawnEntity == nil then
                                print(tostring(Encounter) .. " Invalid spawn entity detected. Exiting.")
                                return ""
                        end
                        --Store in Critters{} so it/they can be properly despawned later as needed.
                        local newID = spawnCritter(SpawnEntity, EncPos, EncRot)
                        self.MyCritters[Counter] = newID
                        --print("My critter count (" .. self.EncounterName .. ") " .. self:AddCritter(newID))
                end
                self:SetState(STATUS_RUNNING_ACTIVE)    --This encounter is now alive
        end

        return ""
end

--Removes owned encounter from the world
--If override is true, even a 100% SP critter will despawn
EncounterManager.Despawn = function (self, override)
        local base, adj = self:GetSpawnProbability()

        if override == true or (base + adj) < 100 then
                self:SetState(STATUS_RUNNING_DESPAWN)
                local totCritters = #self.MyCritters
                
                if totCritters <= 0 then
                        self:SetState(STATUS_RUNNING_IDLE)
                        return ""       -- nothing to despawn
                end

                for i = 1, totCritters do
                        MapInstance.RemoveNpc(self.MyCritters[i])
                end
                --reset the table
                self.MyCritters = nil --Make sure we're cleared
                self.MyCritters = {} --and ready for a new batch
                self:SetState(STATUS_RUNNING_IDLE)
        end        
end

--[[
        ESSENTIALS
        NOTE: Some may be better placed in a Global script,
        CreateEncounterManager will become part of a Zone manager instead.
]]

--Guess!!
function spawnCritter(critter, position, rotation, level, rank, costume)
        local Costume_variation = costume or 1
        local Security_level = level or 0
        local Critter_rank = rank or 0
        return MapInstance.AddEnemy(critter, position, rotation, Costume_variation, tostring(critter), Security_level, tostring(critter), Critter_rank)
end

function spawnCivilian(critter, position, rotation, costume)
        local Costume_variation = costume or 1
        MapInstance.AddNpc(critter, position, rotation, Costume_variation, tostring(critter))
        --return MapInstance.AddNpc(critter, position, rotation, Costume_variation, tostring(critter))
end

--Returns a table of subgroups found in supplied container group.
function GetGroupContainers(Container)
        local Groups = {}
          for k,v in pairs(Container) do
              if not tonumber(k) then     --ignoring numeric index values
                table.insert(Groups, k)   --k is therefore (hypothetically) a grpXXXX
              end
          end
        return Groups
end

--Returns a count of subgroups found in supplied container group.
function GetGroupContainersCount(Container)
        local Count = 0
          for k,v in pairs(Container) do
              if not tonumber(k) then     --ignoring numeric index values
                Count = Count + 1
              end
          end
        return Count
end

--Creates a new Encounter Generator and adds it to the provided table
function CreateEncounterManager(pool, position)
        if type(pool) ~= "table" then
                print("Invalid pool. Exiting.")
                return ""
        end

        if position == nil then
                position = vec3.new(0, 0, 0)
        end

        local a = EncounterManager:new{Position = position}
                table.insert(pool, a)

        return a
end
