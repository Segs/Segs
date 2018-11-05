printDebug('global script loading...')

local enhancements = {
    'Generic_Damage', 'Generic_Accuracy', 'Generic_Resistance', 'Generic_Defense',
    'Generic_Endurance_reduction', 'Generic_Recharge', 'Generic_Health', 'Generic_Fly',
    'Generic_Hold', 'Generic_Run'
}

local inspirations = { 'Insight', 'Enrage', 'Luck', 'Catch_A_Breath', 'Respite',
 'Discipline', 'Awaken'

}
-- Wrapper for Character class


Player = {}
function Player.SetHp(hp)
    Character.setHp(client, hp)
end

function Player.GiveHp (hp)
    Character.giveHp(client, hp)
end

function Player.SetEnd(endurance) -- end is keyword in Lua
    Character.setEnd(client, endurance)
end

function Player.GiveEnd(endurance)
    Character.giveEnd(client, endurance)
end

function Player.SetXp(xp)
    Character.setXp(client, xp)
end

function Player.GiveXp(xp)
    Character.giveXp(client, xp)
end

function Player.SetDebt(debt)
    Character.setDebt(client, debt)
end

function Player.GiveDebt(debt)
    Character.giveDebt(client, debt)
end

function Player.SetInf(inf)
    Character.setInf(client, inf)
end

function Player.GiveInf(inf)
    Character.giveInf(client, inf)
end

function Player.AddUpdateContact(contact)
    Character.addUpdateContactList(client, contact)
end

function Player.AddUpdateTask(task)
    Character.addTask(client, task)
end

function Player.SelectTask(task)
    Character.selectTask(client, task)
end

function Player.LevelUp()
    Character.levelUp(client)
end

function Player.SetTitle(title)
    Character.setTitle(client, title)
end

--Just for testing
function Player.GiveRandomInsp()
    local randomIndex = math.random(1, 7)
    printDebug(tostring(randomIndex))
    Character.giveInsp(client, inspirations[randomIndex])
end

function Player.GiveRandomEnhancement(level)
    local randomIndex = math.random(1, 10)
    printDebug(tostring(randomIndex))
    Character.giveEnhancement(client, enhancements[randomIndex], level)
end




  --Global Helper Functions
function round (num, numDecimalPlaces)
    local mult = 10^(numDecimalPlaces or 0)
    return math.floor(num * mult + 0.5) / mult
  end

  function roundToString(num, numDecimalPlaces)
    return tonumber(string.format("%." .. (numDecimalPlaces or 0) .. "f", num))
  end

function FindContactByName(item)
    local contact = false
    printDebug("Item to find: " .. item)
   for key, value in pairs(vContacts) do
        printDebug(value.name)
        if value.name == item then
            contact = value
            break
        end
    end
    return contact
end

function FindContactByNpcId (npcId)
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

function FindTaskByTaskIdx (taskIdx)
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

function UpdateTasksForZone(zone)
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


printDebug('global script loaded')

--Notes for scriping
-- 112, 16, -216
-- 11 buttons is the max you can have displayed at once. 
--  Anymore and the client will crash
--[[
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
    {"CONTACTLINK_CHOOSE_TITLE"         ,0x19},
    {"CONTACTLINK_GOTOTAILOR"           ,0x1A},]]