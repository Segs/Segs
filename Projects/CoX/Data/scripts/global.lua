printDebug('Loading global.lua...')

--Testing objects
local enhancements = {
    'Generic_Damage', 'Generic_Accuracy', 'Generic_Resistance', 'Generic_Defense',
    'Generic_Endurance_reduction', 'Generic_Recharge', 'Generic_Health', 'Generic_Fly',
    'Generic_Hold', 'Generic_Run'
}

local inspirations = { 'Insight', 'Enrage', 'Luck', 'Catch_A_Breath', 'Respite',
 'Discipline', 'Awaken'

}

local tempPowers = {
    'Jump_Pack', 'Thermite_Cannon', 'Dimensional_Shield', 'EMP_Glove',
    'Cryoprojection_Bracers'
}

--Task Functions
Tasks = {};
function Tasks.FindTaskByTaskIdx (taskIdx)
    local task = false
    printDebug("Task to find: " .. taskIdx)
    if vTaskList ~= nil then
        for key, value in pairs(vTaskList) do
            printDebug(tostring(value.taskIdx))
            if value.taskIdx == taskIdx then
                printDebug("Task found")
                task = value
                break
            end
        end
    end
    return task
end

function Tasks.UpdateTasksForZone(zone)
    printDebug("Zone to find tasks for: " .. zone)
    if vTaskList ~= nil then
        for key, value in pairs(vTaskList) do
            printDebug(value.location.mapName)
            if value.location.mapName == zone then
                printDebug("zone found")
                value.boardTrain = false
                Player.AddUpdateTask(value)
                break
            else
                value.boardTrain = true
                Player.AddUpdateTask(value)
            end
        end
    end
end
--End Task Functions



  --Global Helper Functions
function round (num, numDecimalPlaces)
    local mult = 10^(numDecimalPlaces or 0)
    return math.floor(num * mult + 0.5) / mult
  end

  function roundToString(num, numDecimalPlaces)
    return tonumber(string.format("%." .. (numDecimalPlaces or 0) .. "f", num))
  end





-- CONTACT Helpers
Contacts = {};
function Contacts.SpawnContacts(zone)
    printDebug("Spawning contacts for: " .. zone)
    if contactsForZone ~= nil then
        for key, value in pairs(contactsForZone) do
            printDebug(value.name)
            local spawning = false
            if(value.spawned ~= true) then
                value.expected = true
                spawning = true
                MapClientSession.AddNpc(value.model, value.location, value.variation, value.name)
            end

            if(spawning == true) then
                break
            end

        end
    end
end

function Contacts.SpawnedContact(id)
    printDebug("SpawnedContact id: " .. tostring(id))
    if contactsForZone ~= nil then
        for key, value in pairs(contactsForZone) do
            printDebug(value.name)
            local found = false
            if(value.expected == true) then
                printDebug(value.name .. ' is expected.')
                value.entityId = id
                value.spawned = true
                value.expected = false
                found = true
            end

            if(found == true) then
                break
            end
        end
    end
end

function Contacts.SetContactDialogsWithHeroName(name)
    printDebug("SetContactDialogsWithHeroName: " .. name)
    if contactsForZone ~= nil then
        for key, value in pairs(contactsForZone) do
            printDebug(value.name)
            if(value.createContactDialogsWithHeroName ~= nil) then
                value.createContactDialogsWithHeroName(name)
            end
        end
    end
end

function Contacts.OpenContactDialog(id)
    printDebug("OpenContactDialog id: " .. tostring(id))
    local isContact = false
    if contactsForZone ~= nil then
        for key, value in pairs(contactsForZone) do
            printDebug(value.name)
            if(value.entityId == id) then
                if(Contacts.ContactAvailable(value)) then
                    -- any custom contact setup stuff
                    value.startDialogs()
                    isContact = true
                end
            end
        end
    end
    return isContact
end

function Contacts.ContactAvailable(contact)
    local isAvailable = false
    local currentLevel = 50 -- TODO Get hero level, Maybe check standing with other contact?

    if(contact.minLevel <= currentLevel) then
        isAvailable = true
    end
    return isAvailable
end

function Contacts.FindContactByName(item)
    local contact = false
    printDebug("Contact to find: " .. item)

    if vContacts ~= nil then
        for key, value in pairs(vContacts) do
         printDebug(value.name)
            if (value.name == item) then
                printDebug("Contact found: " .. value.name)
                contact = value
                break
            end
        end
    end
    return contact
end

function Contacts.FindContactByNpcId (npcId)
    local contact = false
    printDebug("NpcId to find: " .. npcId)
    if vContacts ~= nil then
        for key, value in pairs(vContacts) do
            printDebug(tostring(value.npcId))
            if value.npcId == npcId then
                printDebug("NpcId found")
                contact = value
             break
             end
        end
    end
    return contact
end

--End CONTACT HELPERS


--Just for testing
function Player.GiveRandomInsp()
    local randomIndex = math.random(1, 7)
    printDebug(tostring(randomIndex))
    Player.GiveInsp(inspirations[randomIndex])
end

function Player.GiveRandomEnhancement(level)
    local randomIndex = math.random(1, 10)
    printDebug(tostring(randomIndex))
    Player.GiveEnhancement(enhancements[randomIndex], level)
end

function Player.GiveRandomTempPower()
    local randomIndex = math.random(1, 5)
    printDebug(tostring(randomIndex))
    Player.GiveTempPower(tempPowers[randomIndex])
end




printDebug('Finished Loading global.lua')

--[[            Notes for scriping  

To add scripts to lua use include_lua('path') to load each extra script

For contact dialogs, 11 buttons is the max you can have displayed at once. 
 Anymore and the client will crash

        Contact Dialog buttons    
    {"CONTACTLINK_HELLO"                ,1},
    {"CONTACTLINK_MAIN"                 ,2},
    {"CONTACTLINK_BYE"                  ,3},
    {"CONTACTLINK_MISSIONS"             ,4},
    {"CONTACTLINK_LONGMISSION"          ,5},
    {"CONTACTLINK_SHORTMISSION"         ,6},
    {"CONTACTLINK_ACCEPTLONG"           ,7},
    {"CONTACTLINK_ACCEPTSHORT"          ,8},
    {"CONTACTLINK_INTRODUCE"            ,9},
    {"CONTACTLINK_INTRODUCE_CONTACT1"   ,0x0A},
    {"CONTACTLINK_INTRODUCE_CONTACT2"   ,0x0B},
    {"CONTACTLINK_ACCEPT_CONTACT2"      ,0x0D},
    {"CONTACTLINK_ACCEPT_CONTACT2"      ,0x0D},
    {"CONTACTLINK_GOTOSTORE"            ,0x0E},
    {"CONTACTLINK_TRAIN"                ,0x0F},
    {"CONTACTLINK_WRONGMODE"            ,0x10},
    {"CONTACTLINK_DONTKNOW"             ,0x11},
    {"CONTACTLINK_NOTLEADER"            ,0x12},
    {"CONTACTLINK_BADCELLCALL"          ,0x13},
    {"CONTACTLINK_ABOUT"                ,0x14},
    {"CONTACTLINK_IDENTIFYCLUE"         ,0x15},
    {"CONTACTLINK_NEWPLAYERTELEPORT_AP" ,0x16},
    {"CONTACTLINK_NEWPLAYERTELEPORT_GC" ,0x17},
    {"CONTACTLINK_FORMTASKFORCE"        ,0x18},
    {"CONTACTLINK_GOTOTAILOR"           ,0x1A},
  ]]

  --[[ Respawn Locations

  Hospital_Entrance
  Hospital_Exit

  Gurney
  spn_hosp_lobby01
  spn_hosp_lobby02
  spn_hosp_lobby03
    
  spn_hosp_fl1_01
  spn_hosp_fl2
  spn_hosp_fl3
  ]]

  
--[[ Revive Levels
0 = FULL:
1 = AWAKEN:
2 = BOUNCE_BACK:
3 = RESTORATION:
4 = IMMORTAL_RECOVERY:
5 = REGEN_REVIVE:
    ]]

  --nocoll 1    No Clip

  --[[
      FloatingMessage values

    FloatingMsg_NotEnoughEndurance  = 0,
    FloatingMsg_OutOfRange          = 1,
    FloatingMsg_Recharging          = 2,
    FloatingMsg_NoEndurance         = 3,
    FloatingMsg_Leveled             = 4,
    FloatingMsg_FoundClue           = 5,
    FloatingMsg_FoundEnhancement    = 6,
    FloatingMsg_FoundInspiration    = 7,
    FloatingMsg_MissionComplete     = 8,
    FloatingMsg_TaskForceComplete   = 9,
    FloatingMsg_MissionFailed       = 10,
  ]]