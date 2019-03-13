--- PEREZ PARK
local spawnOnce = false

-- Called after MOTD for now.
function player_connected(id)
    --Id is player entity Id
    printDebug('player_connected Id: ' .. tostring(id))
   
    if spawnOnce == false then
        spinSpawners()
        RandomSpawn(85)
        spawnOnce = true
    end

    return  ''
end