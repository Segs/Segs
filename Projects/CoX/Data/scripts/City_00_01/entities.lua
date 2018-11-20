--- OUTBREAK
include_lua('luaBot.lua')

-- Called after MOTD for now.
function player_connected(id)
    --Id is player entity Id
    printDebug('player_connected Id: ' .. tostring(id))

    UpdateTasksForZone('OutBreak')

    --Spawn LuaBot  NPCID = 1144
    if not LuaBot.spawned then
        LuaBot.expected = true
        MapClientSession.addNpc(client, 'Jumpbot_02', vec3.new(-90, 0, 170), 1, 'LuaBot')
    end

    -- Dialogs that use heroName need to wait till a function is called by `callFuncWithClientContext` for the heroName variable to be set
    LuaBot.contactDialogs[1] = { 
        message = string.format([[<img src="npc:1144" align="left">Hello, {HeroName}.....bzzt.....I mean %s. I am LuaBot. Here to assist you in testing the
                Lua scripting interface.<br><br>Please select an option below to test.]], heroName),
        buttons =  {
            button1 = {"Player Stats","CONTACTLINK_HELLO"},
            button2 = {"Contacts","CONTACTLINK_MAIN"},
            button3 = {"Tasks","CONTACTLINK_MISSIONS"}, 
            button4 = {"MapMenu","CONTACTLINK_LONGMISSION"}, 
            button5 = {"Clues","CONTACTLINK_SHORTMISSION"}, 
            button6 = {"Force Logout","CONTACTLINK_ACCEPTLONG"}, 
            button7 = {"",""} ,
            button8 = {"Leave","CONTACTLINK_BYE"} ,
        }
    }

    return  ''
end

function npc_added(id)
    printDebug('npc_added Id: ' .. tostring(id))

    -- Add flags to catch the npc you spawned?
    if LuaBot.expected then
        LuaBot.id = id
        LuaBot.expected = false
        LuaBot.spawned = true
    end

    return ''
end

entity_interact = function(id, location)

    if location ~= nil then
        printDebug("entity id " .. tostring(id) .. " location info:  x: " .. tostring(location.x) .. " y: " .. tostring(location.y) .. " z: " .. tostring(location.z))
    else
        printDebug("entity id " .. tostring(id))
    end

    if (id == LuaBot.id) then
        Player.SetActiveDialogCallback(LuaBot.callback) -- Set callback for LuaBot dialog buttons
        MapClientSession.contact_dialog(client, LuaBot.contactDialogs[1].message, LuaBot.contactDialogs[1].buttons)
    end

    return ""
end

dialog_button = function(id) -- Will be called if no callback is set
    printDebug("buttonId: " .. tostring(id)) 

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

