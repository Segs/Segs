  --Global Helper Functions

  printDebug('global script')

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
