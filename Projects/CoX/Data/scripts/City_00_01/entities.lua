--- OUTBREAK
include_lua('luaBot.lua')

-- Called after MOTD for now.
function player_connected(id)
    --Id is player entity Id
    printDebug('player_connected Id: ' .. tostring(id))

    UpdateTasksForZone('OutBreak')
    SpawnContacts('OutBreak')
   
    return  ''
end

function npc_added(id)
    printDebug('npc_added Id: ' .. tostring(id))
    SpawnedContact(id)
    -- Spawn next contact
    SpawnContacts('OutBreak')

    return ''
end

entity_interact = function(id, location)
    SetContactDialogsWithHeroName(heroName)

    if location ~= nil then
        printDebug("entity id " .. tostring(id) .. " location info:  x: " .. tostring(location.x) .. " y: " .. tostring(location.y) .. " z: " .. tostring(location.z))
    else
        printDebug("entity id " .. tostring(id))
    end

    if(OpenContactDialog(id) ~= true) then
        -- Generic NPC
        -- Create generic NPC message script for zone?
        
    end
  
--[[ NPC chat message test
    MapClientSession.npcMessage(client, 'Hello Hero!', id)
    MapClientSession.npcMessage(client, 'What are you doing here?', id)
    ]]
    return ""
end

dialog_button = function(id) -- Will be called if no callback is set
    printDebug("No Callback set! ButtonId: " .. tostring(id)) 

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

revive_ok = function(id)
printDebug("revive Ok. Entity: " .. tostring(id))
Player.Revive(0)
Character.respawn(client, 'Hospital_Entrance')

--[[
Character.respawn(client, 'Hospital_Entrance')
[log.scripts]Debug   : Spawn:  "Hospital_Entrance"  loc x:  46.7871  y:  -174  z:  556.506
[log.scripts]Debug   : Spawn:  "Hospital_Entrance"  loc x:  43.7871  y:  -174  z:  576.006
[log.scripts]Debug   : Spawn:  "Hospital_Entrance"  loc x:  46.7871  y:  -174  z:  573.506
[log.scripts]Debug   : Spawn:  "Hospital_Entrance"  loc x:  43.7871  y:  -174  z:  570.506
[log.scripts]Debug   : Spawn:  "Hospital_Entrance"  loc x:  46.7871  y:  -174  z:  568.006
[log.scripts]Debug   : Spawn:  "Hospital_Entrance"  loc x:  43.7871  y:  -174  z:  565.006
[log.scripts]Debug   : Spawn:  "Hospital_Entrance"  loc x:  46.7871  y:  -174  z:  562.006
[log.scripts]Debug   : Spawn:  "Hospital_Entrance"  loc x:  43.7871  y:  -174  z:  559.506

Character.respawn(client, 'Hospital_Exit') 
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  31.5  y:  2.99976  z:  584
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  31.5  y:  2.99976  z:  580.5
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  31.5  y:  2.99976  z:  576.5
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  31.5  y:  2.99976  z:  572.5
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  31.5  y:  2.99976  z:  568
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  29.5  y:  1.99976  z:  570
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  29.5  y:  1.99976  z:  574.5
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  29.5  y:  1.99976  z:  578.5
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  29.5  y:  1.99976  z:  582.5
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  27.5  y:  0.999756  z:  584.5
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  27.5  y:  0.999756  z:  581
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  27.5  y:  0.999756  z:  577
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  27.5  y:  0.999756  z:  573
[log.scripts]Debug   : Spawn:  "Hospital_Exit"  loc x:  27.5  y:  0.999756  z:  568.5

Character.respawn(client, 'Gurney')
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  85.25  y:  -95.9644  z:  610.335
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  86.4976  y:  -96.0549  z:  552.639
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  68.0659  y:  -95.9644  z:  541.462
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  94.75  y:  -95.9644  z:  558.304
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  104.563  y:  -96  z:  591.677
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  112.375  y:  -95.9644  z:  601.289
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  141.066  y:  -95.9644  z:  610.038
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  159.563  y:  -96  z:  593.677
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  121.063  y:  -95.9644  z:  541.462
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  141.998  y:  -96  z:  558.139
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  149.853  y:  -95.9644  z:  550.849
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  38.8526  y:  -143.964  z:  558.488
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  48.25  y:  -143.964  z:  593.474
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  56.5  y:  -143.964  z:  603.639
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  84.893  y:  -143.964  z:  610.585
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  86.4976  y:  -144.055  z:  552.639
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  67.25  y:  -143.964  z:  541.165
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  94.75  y:  -143.964  z:  558.304
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  103.748  y:  -144  z:  591.974
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  113.127  y:  -143.964  z:  601.724
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  140.645  y:  -143.964  z:  610.151
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  159.563  y:  -144  z:  593.677
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  120.643  y:  -143.964  z:  541.349
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  140.391  y:  -144  z:  557.554
[log.scripts]Debug   : Spawn:  "Gurney"  loc x:  150.248  y:  -143.964  z:  550.665

Character.respawn(client, 'spn_hosp_lobby01')
[log.scripts]Debug   : Spawn:  "spn_hosp_lobby01"  loc x:  115.261  y:  -174  z:  589.463
[log.scripts]Debug   : Spawn:  "spn_hosp_lobby01"  loc x:  113.761  y:  -174  z:  585.963
[log.scripts]Debug   : Spawn:  "spn_hosp_lobby01"  loc x:  115.261  y:  -174  z:  582.963

Character.respawn(client, 'spn_hosp_lobby02')
[log.scripts]Debug   : Spawn:  "spn_hosp_lobby02"  loc x:  114.743  y:  -174  z:  573.461
[log.scripts]Debug   : Spawn:  "spn_hosp_lobby02"  loc x:  113.243  y:  -174  z:  569.961
[log.scripts]Debug   : Spawn:  "spn_hosp_lobby02"  loc x:  114.743  y:  -174  z:  566.961

Character.respawn(client, 'spn_hosp_lobby03')
[log.scripts]Debug   : Spawn:  "spn_hosp_lobby03"  loc x:  114.999  y:  -174  z:  557.424
[log.scripts]Debug   : Spawn:  "spn_hosp_lobby03"  loc x:  113.499  y:  -174  z:  553.924
[log.scripts]Debug   : Spawn:  "spn_hosp_lobby03"  loc x:  114.999  y:  -174  z:  550.924

Character.respawn(client, 'spn_hosp_fl1_01')
[log.scripts]Debug   : Spawn:  "spn_hosp_fl1_01"  loc x:  190  y:  -144  z:  563.568
[log.scripts]Debug   : Spawn:  "spn_hosp_fl1_01"  loc x:  188.5  y:  -144  z:  559.568
[log.scripts]Debug   : Spawn:  "spn_hosp_fl1_01"  loc x:  190  y:  -144  z:  556.068
[log.scripts]Debug   : Spawn:  "spn_hosp_fl1_01"  loc x:  189.5  y:  -144.001  z:  579.5
[log.scripts]Debug   : Spawn:  "spn_hosp_fl1_01"  loc x:  188  y:  -144.001  z:  575.5
[log.scripts]Debug   : Spawn:  "spn_hosp_fl1_01"  loc x:  189.5  y:  -144.001  z:  572
[log.scripts]Debug   : Spawn:  "spn_hosp_fl1_01"  loc x:  190.5  y:  -144  z:  595.15
[log.scripts]Debug   : Spawn:  "spn_hosp_fl1_01"  loc x:  188.5  y:  -144  z:  591.65
[log.scripts]Debug   : Spawn:  "spn_hosp_fl1_01"  loc x:  190.5  y:  -144  z:  588.15

Character.respawn(client, 'spn_hosp_fl2')
[log.scripts]Debug   : Spawn:  "spn_hosp_fl2"  loc x:  191  y:  -96  z:  556.5
[log.scripts]Debug   : Spawn:  "spn_hosp_fl2"  loc x:  191  y:  -96  z:  562.5
[log.scripts]Debug   : Spawn:  "spn_hosp_fl2"  loc x:  189  y:  -96  z:  559.461
[log.scripts]Debug   : Spawn:  "spn_hosp_fl2"  loc x:  191  y:  -96  z:  579
[log.scripts]Debug   : Spawn:  "spn_hosp_fl2"  loc x:  189  y:  -96  z:  575.5
[log.scripts]Debug   : Spawn:  "spn_hosp_fl2"  loc x:  191  y:  -96  z:  572.5
[log.scripts]Debug   : Spawn:  "spn_hosp_fl2"  loc x:  191  y:  -96  z:  595
[log.scripts]Debug   : Spawn:  "spn_hosp_fl2"  loc x:  188.5  y:  -96  z:  591.66
[log.scripts]Debug   : Spawn:  "spn_hosp_fl2"  loc x:  191  y:  -96  z:  588

Character.respawn(client, 'spn_hosp_fl3')
[log.scripts]Debug   : Spawn:  "spn_hosp_fl3"  loc x:  190.5  y:  -48  z:  556
[log.scripts]Debug   : Spawn:  "spn_hosp_fl3"  loc x:  190.5  y:  -48  z:  563.5
[log.scripts]Debug   : Spawn:  "spn_hosp_fl3"  loc x:  188.5  y:  -48  z:  559.606
[log.scripts]Debug   : Spawn:  "spn_hosp_fl3"  loc x:  191  y:  -48  z:  579.5
[log.scripts]Debug   : Spawn:  "spn_hosp_fl3"  loc x:  188.5  y:  -48  z:  575.581
[log.scripts]Debug   : Spawn:  "spn_hosp_fl3"  loc x:  190.5  y:  -48  z:  572
[log.scripts]Debug   : Spawn:  "spn_hosp_fl3"  loc x:  190.5  y:  -48  z:  595.5
[log.scripts]Debug   : Spawn:  "spn_hosp_fl3"  loc x:  188.5  y:  -48  z:  591.66
[log.scripts]Debug   : Spawn:  "spn_hosp_fl3"  loc x:  190.5  y:  -48  z:  588.5

]]


--[[ Revive Levels
0 = FULL:
1 = AWAKEN:
2 = BOUNCE_BACK:
3 = RESTORATION:
4 = IMMORTAL_RECOVERY:
5 = REGEN_REVIVE:
    ]]
    

    return "ok"
end
