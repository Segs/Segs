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

  
function DeepCopy(object)
    local lookup_table = {};
    local function _copy(object)
        if type(object) ~= "table" then
            return object;
        elseif lookup_table[object] then
            return lookup_table[object];
        end
        local new_table = {};
        lookup_table[object] = new_table;
        for index, value in pairs(object) do
            new_table[_copy(index)] = _copy(value);
        end
        return setmetatable(new_table, getmetatable(object));
    end
    return _copy(object);
end
function SecondsToClock(seconds)
    local seconds = tonumber(seconds)
  
    if seconds <= 0 then
      return "00:00:00";
    else
      hours = string.format("%02.f", math.floor(seconds/3600));
      mins = string.format("%02.f", math.floor(seconds/60 - (hours*60)));
      secs = string.format("%02.f", math.floor(seconds - hours*3600 - mins *60));
      return hours..":"..mins..":"..secs
    end
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
                MapInstance.AddNpc(value.model, value.location.coordinates, value.location.orientation, value.variation, value.name);
            end

            if(spawning == true) then
                break
            end
        end
    end
end

function Contacts.ParseContactButton(id)
    local button = false;
    button = ParseContactButton(id);
    printDebug("ParseContactButton: " .. tostring(button));
    if (button == 'Not Found') then
        button = false;
    end
    return button;
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
                if(value.isStore) then
                    value.SetupStore(); --Custom store
                end
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
    printDebug("OpenContactDialog id: " .. tostring(id));
    local isContact = false;
    if contactsForZone ~= nil then
        for key, value in pairs(contactsForZone) do
            printDebug(value.name);
            if(value.entityId == id) then
                if(Contacts.ContactAvailable(value)) then
                    -- any custom contact setup stuff

                    value.startDialogs();
                    isContact = true;
                end
            end
        end
    end
    return isContact;
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
    --printDebug("Contact to find: " .. item)

    if vContacts ~= nil then
        for key, value in pairs(vContacts) do
            --printDebug(value.name)
            if (value.name == item) then
                --printDebug("Contact found: " .. value.name)
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

function Contacts.FindLocationByName(locationName)
    local location = false;
    --printDebug("Location to find: " .. locationName)
    if contactsForZone ~= nil then
        for key, value in pairs(contactsForZone) do
            --printDebug(tostring(value.locations))
            if value.locations ~= nil then
                for k, v in pairs(value.locations) do
                    printDebug(tostring(v.name))
                    if v.name == locationName then
                        location = v;
                        break;
                    end
                end

                if(location ~= false) then
                    break;
                end
             end
        end
    end
    return location;
end

--End CONTACT HELPERS 450 0 771


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


    contact.name = Name in contact list
    contact.currentStanding = Sets the progess bar
    contact.notifyPlayer = false; ?
    contact.npcId = sets the headshot to display in contact list
    contact.contactIdx = ?
    contact.hasLocation = true; Sets if the contact has a location to travel too.
    contact.taskIndex = 0; Sets which task in the task list is tied is from this contact
    contact.locationDescription = Description of contacts location
    contact.location = Destination.new(); create destination object
    contact.location.location = vec3 of where this contact is
    contact.location.name = Name of location where contact is
    contact.location.mapName =  Sets the name of the map where the contact is.
    contact.confidantThreshold = Sets standing where contact becomes confidant
    contact.friendThreshold = Sets standing where contact becomes friend
    contact.completeThreshold = Sets standing where contact ark is complete
    contact.canUseCell = false; Sets if call button is visible for contact
    contact.dialogScreenIdx = 1; Sets dialog screen index. Use for branching dialogs.

        Contact Dialog buttons    
    {"CONTACTLINK_HELLO"                ,1}, 1
    {"CONTACTLINK_MAIN"                 ,2}, 2
    {"CONTACTLINK_BYE"                  ,3}, 3 
    {"CONTACTLINK_MISSIONS"             ,4}, 4
    {"CONTACTLINK_LONGMISSION"          ,5}, 5
    {"CONTACTLINK_SHORTMISSION"         ,6}, 6
    {"CONTACTLINK_ACCEPTLONG"           ,7}, 7
    {"CONTACTLINK_ACCEPTSHORT"          ,8}, 8
    {"CONTACTLINK_INTRODUCE"            ,9}, 9
    {"CONTACTLINK_INTRODUCE_CONTACT1"   ,0x0A}, 10
    {"CONTACTLINK_INTRODUCE_CONTACT2"   ,0x0B}, 11
    {"CONTACTLINK_ACCEPT_CONTACT2"      ,0x0D}, 13
    {"CONTACTLINK_GOTOSTORE"            ,0x0E}, 14
    {"CONTACTLINK_TRAIN"                ,0x0F}, 15
    {"CONTACTLINK_WRONGMODE"            ,0x10}, 16
    {"CONTACTLINK_DONTKNOW"             ,0x11}, 17
    {"CONTACTLINK_NOTLEADER"            ,0x12}, 18
    {"CONTACTLINK_BADCELLCALL"          ,0x13}, 19
    {"CONTACTLINK_ABOUT"                ,0x14}, 20
    {"CONTACTLINK_IDENTIFYCLUE"         ,0x15}, 21
    {"CONTACTLINK_NEWPLAYERTELEPORT_AP" ,0x16}, 22
    {"CONTACTLINK_NEWPLAYERTELEPORT_GC" ,0x17}, 23
    {"CONTACTLINK_FORMTASKFORCE"        ,0x18}, 24
    {"CONTACTLINK_GOTOTAILOR"           ,0x1A}, 26


    Respawn Locations

  Hospital_Entrance
  Hospital_Exit

  Gurney
  spn_hosp_lobby01
  spn_hosp_lobby02
  spn_hosp_lobby03
    
  spn_hosp_fl1_01
  spn_hosp_fl2
  spn_hosp_fl3

  

  Revive Levels
  0 = FULL:
  1 = AWAKEN:
  2 = BOUNCE_BACK:
  3 = RESTORATION:
  4 = IMMORTAL_RECOVERY:
  5 = REGEN_REVIVE:


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

    Chat MessageChannel

    COMBAT         = 1, // COMBAT
    DAMAGE         = 2, // DAMAGE
    SERVER         = 3, // SVR_COM
    NPC_SAYS       = 4, // NPC_SAYS
    VILLAIN_SAYS   = 5, // VILLAIN_SAYS
    REGULAR        = 6, // REGULAR
    PRIVATE        = 7, // Tell/Private
    TEAM           = 8, // Group/Team
    SUPERGROUP     = 9, // SuperGroup
    LOCAL          = 10, // Local
    BROADCAST      = 11, // Shout
    REQUEST        = 12, // Request
    FRIENDS        = 13, // Friendlist
    ADMIN          = 14, // [Admin]{Message}
    USER_ERROR     = 15, // User Errors
    DEBUG_INFO     = 16, // Debug Info
    EMOTE          = 17, // Emotes
    CHAT_TEXT      = 18, // General CHAT
    PROFILE_TEXT   = 19, // Profile Text; unused?
    HELP_TEXT      = 20, // Help Text; unused?
    STD_TEXT       = 21, // Standard Text; unused?



    Location Coor vs MiniMap
    X +num = West 
    X -Num = East
    Z +Num = South
    Z -Num = North

    Y is vertial axis
    Y +Num = Up
    Y -Num = down


    Rotation


    Use the Y axis to rotate in radians
    All entities default to facing south.
    Y +Num moves clockwise
    Y -Num moves counterclockwise
    Example of rotating to face North
    vec3.new(0, 3.14, 0);


    Statistic Ids
    1 HideAndSeek

    Client debug commands

    nocoll 1                  No Clip
    entdebugclient 1          Player info
    loc                       Player location

  End Notes  ]]