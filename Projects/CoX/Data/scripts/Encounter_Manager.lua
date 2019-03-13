--include_lua('Mock_ES.lua')
include_lua('ES_Library_Objects.lua')


--Sends a server message
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

--[[
SpawnerData is a single encounter spawner group with this structure:
<<Spawner Name>>
  <<Makers>>
    <<Pos, Rot>>
<<Properties>>

Anything with a SpawnProbability of 100% should always used that value
and ignore an adjustment if sent, as they were intended to appear in the
world without variance.
]]

--Several of these values should be drawn from a config file.
EncounterManager = {
        ["SpawnerData"] = {},
        ["Critters"] = {},
        ["Position"] = vec3.new(0, 0, 0),
        ["StatusCurrent"] = STATUS_SLEEPING,
        ["StatusPrevious"] = STATUS_SLEEPING,
        ["EncounterIsActive"] = false,
        ["PollingIntervalBase"] = 60.0,
        ["PollingIntervalAdj"] = 0,
        ["PanicDuration"] = 90.0,
        ["PanicInitiators"] = {},
        ["SpawnProbabilityBase"] = 80,
        ["SpawnProbabilityAdj"] = 0,
        ["EncounterName"] = "",
        ["Markers"] = {}
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
        local counter = #self.Critters or 0
        return #self.Critters
end

--Handles state changes for the EM
EncounterManager.SetState = function (self, status)

        --TODO: Validate that the supplied status is valid

        self.StatusPrevious = self.StatusCurrent
        self.StatusCurrent = status

        if status == STATUS_SLEEPING then
              --Do nothing for now
        elseif status == STATUS_RUNNING_IDLE then
              self.EncounterIsActive = false
        elseif status == STATUS_RUNNING_ACTIVE then
              self.EncounterIsActive = true
        elseif status == STATUS_RUNNING_PANIC then
              --Start the panic timer
              --Panic Initiators need to be handled somewhere but likely not here.
              --Probably withhin whatever called this status change
        elseif status == STATUS_RUNNING_DESPAWN then
              --Start despawn sequence
              --That sequence should set status to idle or sleep when complete
        end
end

--Returns a translated status message and allows for further handling if desired.
EncounterManager.GetState = function (self)

        local status = self.StatusCurrent

        if status == STATUS_SLEEPING then
                return "STATUS_SLEEPING"
        elseif status == STATUS_RUNING_IDLE then
                return "STATUS_RUNING_IDLE"
        elseif status == STATUS_RUNNING_ACTIVE then
                return "STATUS_RUNNING_ACTIVE"
        elseif status == STATUS_RUNNING_PANIC then
                return "STATUS_RUNNING_PANIC"
        elseif status == STATUS_RUNNING_DESPAWN then
                return "STATUS_RUNNING_DESPAWN"
        end

        return "STATUS_INVALID" --Something is wrong if we end up here.
end

--Used for manual human interactions
EncounterManager.ReportStatus = function (self)
        local Pos = self.Position
        local alert = "I am at (" .. tostring(Pos.x) .. ", " .. tostring(Pos.y) .. ", " .. tostring(Pos.z) .. ")"
        alert = alert .. "\nMy status is: " .. self:GetState()
        CallOut(alert)
        return ""
end

--Interacts with the Spawndef to create an active encounter
EncounterManager.Spawn = function (self)
        --Can't use os.time() for some reason? This should do?
        math.randomseed(DateTime.SecsSince2000Epoch() * self.Position.x)

        local Encounter = self.EncounterName
        --print("Validating " .. tostring(Encounter))
        --print("EM LOC: " .. self.Position.x .. " " .. self.Position.y .. " " .. self.Position.z)

        if Encounter == nil then
                print("Invalid encounter detected. Exiting.")
                return ""
        end

        --IGNORE FOR NOW
        if Encounter == "_ES_PoliceDrone" then
                print("Not handling drones currently. Exiting.")
                return ""
        end

        local validateSpawnDef = IsSpawnDef(Encounter)

        if validateSpawnDef ~= nil and validateSpawnDef == true then
                local BaseType = GetBaseType(Encounter)
                --Ambush are for missions or special encounters only; exit.
                --Encounter types are for monsters or special encounters; exit.
                if BaseType == nil or 
                BaseType == "Ambush" or
                BaseType == "Encounter" then
                        print("Ambush, Special or NIL detected. Exiting.")
                        return ""  
                end
                --For now, get a random SpawnDef from those available
                local SquadCount = math.random(1, GetCanSpawnCount(Encounter))
                local Variant = {}
                --print("Squad Count: " .. SquadCount)

                Variant = GetVariant(Encounter, SquadCount)
                --print("Variant: " .. tostring(Variant))

                --Step through the template's markers and critters
                for Tmarker, Tcritter in pairs(Variant[BaseType]) do
                        --try to find a marker to match the Tmarker
                        local count = #self.Markers
                        local EncPos
                        local EncRot
                        
                        --this EM is improperly initiatlized or is broken, exit
                        if count == 0 or count == nil then
                                print("EM has no markers. Exiting.")
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
                                print("EM LOC: " .. self.Position.x .. " " .. self.Position.y .. " " .. self.Position.z)
                                print("data: " .. tostring(EncPos.x) .. " " .. tostring(EncPos.y) .. " " .. tostring(EncPos.z))
                                print("EM has bad position data. Exiting.")
                                return ""                        
                        elseif EncRot == nil then
                                print("EM LOC: " .. self.Position.x .. " " .. self.Position.y .. " " .. self.Position.z)
                                print("data: " .. tostring(EncRot.y))
                                print("EM has bad rotation data. Exiting.")
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
                                print("Invalid spawn entity detected. Exiting.")
                                return ""
                        end

                        --We'll need this to return the spawned critter's ID
                        --to store in Critters{}
                        --so that it can be properly despawned later as needed.
                        spawnCritter(SpawnEntity, EncPos, EncRot)
                end
                self:SetState(STATUS_RUNNING_ACTIVE)    --This encounter is now alive
        end

        return ""
end

--[[
        ESSENTIALS
        NOTE: Some may be better placed in a Global script,
        CreateEncounterManager will probably disappear or become some part
        of the Zone manager instead.
]]

--Guess!!
function spawnCritter(critter, position, rotation, level, rank, costume)
        local costume_variation = costume or 1
        local security_level = level or 0
        local critter_rank = rank or 0
        MapInstance.AddEnemy(critter, position, rotation, costume_variation, tostring(critter), security_level, tostring(critter), critter_rank)
        return ""
end

function spawnCivilian(critter, position, rotation, costume)
        local costume_variation = costume or 1
        MapInstance.AddNpc(critter, position, rotation, costume_variation, tostring(critter))
        return ""
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

--[[
        Creates a new Encounter Generator and adds it to the provided table
        Later, a zone manager should be the source of spawning all encounter managers
        for it's map or zone, inherently storing them in the process.
]]
function CreateEncounterManager(pool, position)
        if type(pool) ~= "table" then
                print("Invalid pool. Exiting.")
                return ""
        end

        --[[ May want to re-enable this again?
        if spawner == nil then
                print("Spawner group required. Exiting.")
        end
        ]]

        if position == nil then
                position = vec3.new(0, 0, 0)
        end

        local a = EncounterManager:new{Position = position}
                table.insert(pool, a)

        return a
end
