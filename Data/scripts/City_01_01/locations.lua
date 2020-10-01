
location_visited = function(name, coordinates)
    printDebug("Location name: " .. name .. " Loc X: " .. round(coordinates.x, 4) .. " Y: " .. round(coordinates.y, 4) .. " Z: " .. round(coordinates.z, 4))

    local location = Contacts.FindLocationByName(name)
   
    if(location ~= false) then
        location.action();
    end

    return ""
end

enter_door = function(name, location)
    printDebug("enter_door name: " .. name .. " Loc X: " .. round(coordinates.x, 4) .. " Y: " .. round(coordinates.y, 4) .. " Z: " .. round(coordinates.z, 4))

    local x = round(coordinates.x, 0)
    local y = round(coordinates.y, 0)
    local z = round(coordinates.z, 0)

    printDebug("Rounded Location:  X: " .. tostring(x) .. " Y: " .. tostring(y) .. " Z: " .. tostring(z))

    if(x == 36) then
        Player.ExitDoor(vec3.new(44.4, -174, 575))    

    end
    
    return ""
end
