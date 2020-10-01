-- OUTBREAK

local spawnOnce = false

entity_interact = function(id, location)
    Contacts.SetContactDialogsWithHeroName(heroName)

    if location ~= nil then
        printDebug("entity id " .. tostring(id) .. " location info:  x: " .. tostring(location.x) .. " y: " .. tostring(location.y) .. " z: " .. tostring(location.z))
    else
        printDebug("entity id " .. tostring(id))
    end
    
    if(Contacts.OpenContactDialog(id) ~= true) then
        -- Generic NPC
        -- Create generic NPC message script for zone?
        
    end
  
--[[ NPC chat message test
    MapClientSession.npcMessage(client, 'Hello Hero!', id)
    MapClientSession.npcMessage(client, 'What are you doing here?', id)
    ]]
    return ""
end

-- Called after MOTD for now.
function player_connected(id)
    --Id is player entity Id
    printDebug('player_connected Id: ' .. tostring(id))

    Tasks.UpdateTasksForZone('OutBreak')
    Contacts.SpawnContacts('OutBreak')

    if spawnOnce == false then
        spinSpawners()
        spinPersists()
        --spinCivilians()       --This should load "walking" police drones for Outbreak instead of civies.
        --spinCars()            --Cars shouldn't appear in Outbreak, but they could.
        RandomSpawn(75)
        spawnOnce = true

        --print("Initiating map auto-refresh")
        MapInstance.SetOnTickCallback(contactsForZone.TimeCop.entityId, contactsForZone.TimeCop.onTickCallBack);
        TimeCopMode(true, 75, 180)        
    end

    return  ''
end

function npc_added(id)
    printDebug('npc_added Id: ' .. tostring(id))
    Contacts.SpawnedContact(id)
    -- Spawn next contact
    Contacts.SpawnContacts('OutBreak')

    return ''
end

set_target = function(id)
    print("target id " .. tostring(id))
    Character.faceEntity(client.m_ent, id)
    return ""
end

dialog_button = function(id) -- Will be called if no callback is set
    printDebug("No Callback set! ButtonId: " .. tostring(id))

    return ""
end

contact_call = function(contactIndex)
    printDebug("Contact Call. contactIndex: " .. tostring(contactIndex))

    return ""
end

revive_ok = function(id)
    printDebug("revive Ok. Entity: " .. tostring(id))
    Character.respawn(client, 'Gurney'); -- Hospital
    Player.Revive(0);

    return "ok"
end

