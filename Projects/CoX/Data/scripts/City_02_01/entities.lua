--- Steel Canyon


-- Called after MOTD for now.
function player_connected(id)
    --Id is player entity Id
    printDebug('player_connected Id: ' .. tostring(id))
   
    return  ''
end

function npc_added(id)

    return ''
end

function entity_interact(id, location)
    printDebug('entity_interact Id: ' .. tostring(id))
    return ""
end

function dialog_button(id) -- Will be called if no callback is set
    printDebug("No Callback set! ButtonId: " .. tostring(id)) 

    return ""
end

function contact_call(contactIndex)
    printDebug("Contact Call. contactIndex: " .. tostring(contactIndex))

    return ""
end

function revive_ok(id)
    printDebug("revive Ok. Entity: " .. tostring(id))

    return ""
end
