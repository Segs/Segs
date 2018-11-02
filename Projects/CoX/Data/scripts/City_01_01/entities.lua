--- ATLAS PARK

LuaBot2 = {}
LuaBot2.spawned = false;
LuaBot2.contactDialogs = {}


-- Called after MOTD for now.
function player_connected(id)
    --Id is player entity Id
    printDebug('player_connected Id: ' .. tostring(id))

    UpdateTasksForZone('Atlas Park') -- Set tasks in list of this zone to not say board train?
    
    --Spawn LuaBot  NPCID = 1144
    if not LuaBot2.spawned then
        LuaBot2.expected = true
        MapClientSession.addNpc(client, 'Jumpbot_02', vec3.new(112, 16, -216), 1, 'LuaBot2')
    end

    -- Dialogs that use heroName need to wait till a function is called by `callFuncWithClientContext` to update the heroName variable
        LuaBot2.contactDialogs[1] = { 
        message = string.format([[<img src="npc:1144" align="left">Ah, %s. I see you have the test bit.<br><br>Please hand it to me.<br>]], heroName),
        buttons =  {
            button1 = {"Deliver Bit","CONTACTLINK_HELLO"},
            button2 = {"",""} ,
            button3 = {"Leave","CONTACTLINK_BYE"} ,
        }
    }
        LuaBot2.contactDialogs[2] = { 
            message = string.format([[<img src="npc:1144" align="left">Thank you, %s. I can send you back to my counterpart when you are ready.<br>]], heroName),
            buttons =  {
                button1 = {"MapMenu","CONTACTLINK_HELLO"},
                button2 = {"",""} ,
                button3 = {"Leave","CONTACTLINK_BYE"} ,
            }
        }
    
    return  ''
end

function npc_added(id)
    printDebug('npc_added Id: ' .. tostring(id))

    -- Add flags to catch the npc you spawned?
    if LuaBot2.expected then
        LuaBot2.id = id
        LuaBot2.expected = false
        LuaBot2.spawned = true
    end

    return ''
end

entity_interact = function(id, location)
    local LuaBotContact = FindContactByNpcId(1144)
    if location ~= nil then
        printDebug("entity id " .. tostring(id) .. " location info:  x: " .. tostring(location.x) .. " y: " .. tostring(location.y) .. " z: " .. tostring(location.z))
    else
        printDebug("entity id " .. tostring(id))
    end

    if (id == LuaBot2.id and LuaBotContact.currentStanding == 0) then
        MapClientSession.contact_dialog(client, LuaBot2.contactDialogs[1].message, LuaBot2.contactDialogs[1].buttons)
    elseif (id == LuaBot2.id and LuaBotContact.currentStanding == 1) then
        MapClientSession.contact_dialog(client, LuaBot2.contactDialogs[2].message, LuaBot2.contactDialogs[2].buttons)
    end

    return ""
end

dialog_button = function(id)
    printDebug("buttonId: " .. tostring(id))
    local LuaBotContact = FindContactByNpcId(1144)
    local LuaBotMission = FindTaskByTaskIdx(0) -- Need better way to search. Could create out own Id for tasks

    if (LuaBot2.id ~= nil and LuaBotContact ~= false and LuaBotMission ~= false) then
        if (LuaBotContact.currentStanding == 0) then
            if (id == 1) then
                LuaBotContact.currentStanding = 1
                Player.AddUpdateContact(LuaBotContact)
                -- Misson Complete sound?
                LuaBotMission.state = "Return to LuaBot in Outbreak"
                LuaBotMission.isComplete = true
                Player.AddUpdateTask(LuaBotMission)
                MapClientSession.contact_dialog(client, LuaBot2.contactDialogs[2].message, LuaBot2.contactDialogs[2].buttons)
            end
        elseif (LuaBotContact.currentStanding == 1) then
            if (id == 1) then
                MapClientSession.mapMenu(client)
            end
        end
    end

    return ""
end


--Not used but needed for scripting Engine
set_target = function(id) 
    printDebug("target id " .. tostring(id))
    
    return ""
end

contact_call = function(contactIndex)
    printDebug("Contact Call. contactIndex: " .. tostring(contactIndex))

    return ""
end

