printDebug('Loading contacts.lua...')

-- Contact Scripts
include_lua('luaBot.lua')

-- End Contact scripts


contacts = {}

function SpawnContacts(zone)
    printDebug("Spawning contacts for: " .. zone)
    if contacts ~= nil then
        for key, value in pairs(contacts) do
            printDebug(value.name)
            local spawning = false
            if(value.spawned ~= true) then
                value.expected = true
                spawning = true
                MapClientSession.addNpc(client, value.model, value.location, value.variation, value.name)
            end

            if(spawning == true) then
                break
            end

        end
    end
end

function SpawnedContact(id)
    printDebug("SpawnedContact id: " .. tostring(id))
    if contacts ~= nil then
        for key, value in pairs(contacts) do
            printDebug(value.name)
            local found = false
            if(value.expected == true) then
                printDebug(value.name .. ' is expected.')
                value.entityId = id
                value.spawned = true
                value.expected = false
                found = true
            end

            if(found == true) then
                break
            end
        end
    end
end

function SetContactDialogsWithHeroName(name)
    printDebug("SetContactDialogsWithHeroName: " .. name)
    if contacts ~= nil then
        for key, value in pairs(contacts) do
            printDebug(value.name)
            if(value.createContactDialogsWithHeroName ~= nil) then
                value.createContactDialogsWithHeroName(name)
            end
        end
    end
end

function OpenContactDialog(id)
    printDebug("OpenContactDialog id: " .. tostring(id))
    local isContact = false
    if contacts ~= nil then
        for key, value in pairs(contacts) do
            printDebug(value.name)
            if(value.entityId == id) then
                if(ContactAvailable(value)) then
                    -- any custom contact setup stuff
                    value.startDialogs()
                    isContact = true
                end
            end
        end
    end
    return isContact
end

function ContactAvailable(contact)
    local isAvailable = false
    local currentLevel = 50 -- TODO Get hero level

    if(contact.minLevel <= currentLevel) then
        isAvailable = true
    end
    return isAvailable
end

function FindContactByName(item)
    local contact = false
    printDebug("Contact to find: " .. item)

    if vContacts ~= nil then
        for key, value in pairs(vContacts) do
         printDebug(value.name)
            if (value.name == item) then
                printDebug("Contact found: " .. value.name)
                contact = value
                break
            end
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

printDebug('Finished Loading contacts.lua')