--- Terra Volta

function location_visited(name, location)
   printDebug("Location name: " .. name .. " Loc X: " .. round(location.x, 4) .. " Y: " .. round(location.y, 4) .. " Z: " .. round(location.z, 4))
  
    return ""
end

function enter_door(name, location)
    printDebug("enter_door name: " .. name .. " Loc X: " .. round(location.x, 4) .. " Y: " .. round(location.y, 4) .. " Z: " .. round(location.z, 4))

    local x = round(location.x, 0)
    local y = round(location.y, 0)
    local z = round(location.z, 0)

    printDebug("Rounded Location:  X: " .. tostring(x) .. " Y: " .. tostring(y) .. " Z: " .. tostring(z))
    
    return ""
end
