printDebug('luaBot script loading...')

local LuaBot = {}
LuaBot.spawned = false;
LuaBot.name = 'LuaBot'
LuaBot.model = 'Jumpbot_02'
LuaBot.location = vec3.new(-90, 0, 170)
LuaBot.variation = 1
LuaBot.expected = false
LuaBot.entityId = nil
LuaBot.minLevel = 0 -- Level requirement?
LuaBot.contactDialogs = {}
LuaBot.contacts = {}
LuaBot.tasks = {}

LuaBot.clue = Clue.new()
LuaBot.clue.name = "LuaBot Test Clue"
LuaBot.clue.displayName = "LuaBot Clue"
LuaBot.clue.detail = "Test clue from LuaBot"
LuaBot.clue.iconFile = "icon" -- Client file name?

LuaBot.souvenir = Souvenir.new()
LuaBot.souvenir.name = "LuaBot souvenir"
LuaBot.souvenir.description = "A Souvenir from LuaBot"
LuaBot.souvenir.icon = "icon" -- unknown?

LuaBot.createContactDialogsWithHeroName = function(heroName)
    contactsForZone.LuaBot.contactDialogs[1] = { 
        message = string.format([[<img src="npc:1144" align="left">Hello, {HeroName}.....bzzt.....I mean %s. I am LuaBot. Here to assist you in testing the
                Lua scripting interface.<br><br>Please select an option below to test.]], heroName),
        buttons =  {
            button1 = {"Player Stats","CONTACTLINK_HELLO"},
            button2 = {"Contacts","CONTACTLINK_MAIN"},
            button3 = {"Tasks","CONTACTLINK_MISSIONS"}, 
            button4 = {"MapMenu","CONTACTLINK_LONGMISSION"}, 
            button5 = {"Clues","CONTACTLINK_SHORTMISSION"}, 
            --button6 = {"Force Logout","CONTACTLINK_ACCEPTLONG"}, 
            button7 = {"",""} ,
            button8 = {"Leave","CONTACTLINK_BYE"} ,
        }
    }

    contactsForZone.LuaBot.contactDialogs[9] = {
        message = string.format([[<img src="npc:1144" align="left"><br>Ah %s, I see you have the test bit.<br>
                    Please hand the bit to me now.<br>]], heroName),
        buttons = {
            button01 = {"Deliver Bit","CONTACTLINK_MISSIONS"},
            --button02 = {"Mapmenu","CONTACTLINK_MAIN"},
            --button03 = {"Update Task Name","CONTACTLINK_MISSIONS"}, 
            --button04 = {"Set Task to another zone","CONTACTLINK_LONGMISSION"} 
            --button05 = {"Complete Task","CONTACTLINK_SHORTMISSION"} ,
            --button06 = {"Remove Task","CONTACTLINK_ACCEPTLONG"} ,
            button07 = {"",""},
            button08 = {"Leave","CONTACTLINK_BYE"}
        }
    }

end

LuaBot.startDialogs = function()
    Player.SetActiveDialogCallback(contactsForZone.LuaBot.callback)
    
    local contactStatus = Contacts.FindContactByName("LuaBot")
    if(contactStatus ~= false and contactStatus.currentStanding < 6) then
        contactStatus.currentStanding = 0
        contactStatus.locationDescription = "Outbreak"
        contactStatus.location = Destination.new()
        contactStatus.location.location = vec3.new(-90, 0, 170)
        contactStatus.location.name = "Outbreak"
        contactStatus.location.mapName = "Outbreak"
        Player.AddUpdateContact(contactStatus)
    elseif(contactStatus == false) then
        contactStatus = contactsForZone.LuaBot.CreateContact()
        Player.AddUpdateContact(contactStatus)
    end

    local testTask = Tasks.FindTaskByTaskIdx(0);
    if(contactStatus.currentStanding == 6 and testTask ~= false) then -- Test bit
        if(testTask.location.mapName == "Outbreak") then
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[9].message, contactsForZone.LuaBot.contactDialogs[9].buttons)
        end
    elseif (contactStatus.currentStanding == 7 and testTask ~= false) then -- return after test bit
        if(testTask.location.mapName == "Outbreak" and testTask.isComplete) then
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[11].message, contactsForZone.LuaBot.contactDialogs[11].buttons)
        else
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[10].message, contactsForZone.LuaBot.contactDialogs[10].buttons)
            
        end
    else
        MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[1].message, contactsForZone.LuaBot.contactDialogs[1].buttons)
    end
end

LuaBot.callback = function(id)
    printDebug('LuaBotCallback id:' .. tostring(id));
    local contactStatus = Contacts.FindContactByName("LuaBot");
    local testContactStatus = Contacts.FindContactByName("LuaBot Test");
    local testTask = Tasks.FindTaskByTaskIdx(0);

    if (contactStatus ~= false) then
        if(contactsForZone.LuaBot.settingTitle) then -- Title selection buttons trigger dialog_button
            LuaBot.settingTitle = false
            contactStatus.currentStanding = 0
            if(id ~= 0) then
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[1].message, contactsForZone.LuaBot.contactDialogs[1].buttons)
            end
        elseif(contactStatus.currentStanding == 0) then -- using mode to reuse button Ids
            if (id == 1) then
                printDebug("LuaBot moving to stat mode: " .. tostring(id))
                contactStatus.currentStanding = 1 -- Player stats
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[2].message, contactsForZone.LuaBot.contactDialogs[2].buttons)
                
                local LuaBotMission = Tasks.FindTaskByTaskIdx(0)
                if(LuaBotMission ~= false) then
                    local task = Task.new() -- empty task
                    Player.AddUpdateTask(task)
                end
            elseif (id == 2) then
                printDebug("LuaBot moving to contact mode: " .. tostring(id))
                contactStatus.currentStanding = 3 -- Contacts
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[4].message, contactsForZone.LuaBot.contactDialogs[4].buttons)
            elseif (id == 3) then
                contactStatus.currentStanding = 0
            elseif (id == 4) then
                printDebug("LuaBot moving to task mode: " .. tostring(id))
                contactStatus.currentStanding = 4 -- Tasks
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[6].message, contactsForZone.LuaBot.contactDialogs[6].buttons)
                Player.AddUpdateContact(contactStatus)
            elseif (id == 5) then
                MapClientSession.MapMenu(client)
            elseif (id == 6) then -- Clue/Souvenir
                printDebug("LuaBot moving to clue/souvenir mode: " .. tostring(id))
                contactStatus.currentStanding = 5
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[7].message, contactsForZone.LuaBot.contactDialogs[7].buttons)
            end
           
        elseif (contactStatus.currentStanding == 1) then -- Stats Page 1
            printDebug("LuaBot in stat mode: " .. tostring(id))
            if (id == 1) then
                Player.SetHp(1)
            elseif (id == 2) then
                Player.GiveHp(99999) -- Should be max
            elseif (id == 3) then
                contactStatus.currentStanding = 0
            elseif (id == 17) then
                Player.SetEnd(1)
            elseif (id == 4) then
                Player.GiveEnd(100)
            elseif (id == 5) then
                Player.GiveXp(1000)
            elseif (id == 6) then
                Player.GiveDebt(1000)
            elseif (id == 7) then
                Player.GiveInf(1000)
            elseif (id == 15) then
                --Train/Level
                Player.LevelUp()
            elseif (id == 9) then
                --reset stats
                Player.GiveHp(99999)
                Player.GiveEnd(100)
                Player.SetXp(0)
                Player.SetDebt(0)
                Player.SetInf(0)
            elseif (id == 16 or id == 3) then
                -- Back
                contactStatus.currentStanding = 0
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[1].message, contactsForZone.LuaBot.contactDialogs[1].buttons)
            elseif (id == 18) then
                --More 
                contactStatus.currentStanding = 2
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[3].message, contactsForZone.LuaBot.contactDialogs[3].buttons)
            end
        elseif (contactStatus.currentStanding == 2) then -- Stats Page 2
            if (id == 1) then
                Player.GiveRandomInsp()
            elseif (id == 2) then
                Player.GiveRandomEnhancement(1)
            elseif (id == 3) then
                contactStatus.currentStanding = 0
            elseif (id == 25) then
                --Since LevelUp closes all dialogs, we need to reset current status
                contactStatus.currentStanding = 0
                Player.OpenTitleMenu()
                LuaBot.settingTitle = true
            elseif (id == 5) then
                Player.GiveRandomTempPower()
            elseif (id == 16 or id == 3) then
                -- Back
                contactStatus.currentStanding = 1
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[2].message, contactsForZone.LuaBot.contactDialogs[2].buttons)
            end

        elseif (contactStatus.currentStanding == 3) then -- Contacts
            printDebug("LuaBot in contact mode: " .. tostring(id))
            if (id == 1) then
                testContactStatus = contactsForZone.LuaBot.CreateTestContact()
                Player.AddUpdateContact(testContactStatus)
            elseif (id == 2) then
                if(testContactStatus ~= false and testContactStatus.currentStanding < 7) then
                    testContactStatus.currentStanding = testContactStatus.currentStanding + 1
                    Player.AddUpdateContact(testContactStatus)
                end
            elseif (id == 3) then
                contactStatus.currentStanding = 0
            elseif (id == 4) then
                if(testContactStatus ~= false and testContactStatus.currentStanding > 0) then
                    testContactStatus.currentStanding = testContactStatus.currentStanding - 1
                    Player.AddUpdateContact(testContactStatus)
                end
            elseif (id == 5) then
                if(testContactStatus ~= false) then
                    testContactStatus.canUseCell = true
                    Player.AddUpdateContact(testContactStatus)
                end
            elseif (id == 6) then
                if(testContactStatus ~= false) then
                    testContactStatus.canUseCell = false
                    Player.AddUpdateContact(testContactStatus)
                end
            elseif (id == 7) then
                --reset/Remove contact
                if(testContactStatus ~= false) then
                    Player.RemoveContact(testContactStatus);
                end 
            elseif (id == 9) then
                contactStatus.currentStanding = 0
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[1].message, contactsForZone.LuaBot.contactDialogs[1].buttons)
            end
        elseif (contactStatus.currentStanding == 4) then -- Tasks
            printDebug("LuaBot in task mode: " .. tostring(id))
            if (id == 1) then
                local task = contactsForZone.LuaBot.CreateTask()
                Player.AddUpdateTask(task)
            elseif (id == 2) then
                if(testTask ~= false) then
                    Player.SelectTask(testTask)
                end
            elseif (id == 3) then
                contactStatus.currentStanding = 0
            elseif (id == 4) then
                if(testTask ~= false) then
                    testTask.description = 'Task Updated - Talked to LuaBot'
                    Player.AddUpdateTask(testTask)
                end
            elseif (id == 5) then
                if(testTask ~= false) then
                    testTask.boardTrain = true
                    testTask.location.name  = "Atlas park"
                    testTask.location.mapName = "Atlas park"
                    Player.AddUpdateTask(testTask)
                end
            elseif (id == 6) then
                if(testTask ~= false) then
                    testTask.isComplete = true
                    testTask.state = 'Return to LuaBot'
                    Player.AddUpdateTask(testTask)
                end
            elseif (id == 7) then
                local task = Task.new() -- empty task
                Player.AddUpdateTask(task)
            elseif (id == 8) then
                -- Task to LuaBot in Atlas
                testTask = contactsForZone.LuaBot.CreateAtlasTask()
                contactStatus.currentStanding = 6
                Player.AddUpdateTask(testTask)
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[8].message, contactsForZone.LuaBot.contactDialogs[8].buttons)
            elseif (id == 9) then
                --Back
                --contacts.LuaBot.CreateTask()
                contactStatus.currentStanding = 0
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[1].message, contactsForZone.LuaBot.contactDialogs[1].buttons)
            end
        elseif (contactStatus.currentStanding == 5) then -- Clue/Souvenirs
            if (id == 1) then
                --add clue
                Player.AddClue(contactsForZone.LuaBot.clue);
                MapClientSession.SendFloatingInfo(5);
            elseif (id == 2) then
                --add souvenir
                Player.AddSouvenir(contactsForZone.LuaBot.souvenir);
                MapClientSession.SendFloatingInfo(7); -- Inspiration sound.
            elseif (id == 4) then -- remove Clue
                Player.RemoveClue(contactsForZone.LuaBot.clue)
            elseif (id == 5) then -- remove Souvenir
                Player.RemoveSouvenir(contactsForZone.LuaBot.souvenir);
            elseif (id == 9) then
                contactStatus.currentStanding = 0
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[1].message, contactsForZone.LuaBot.contactDialogs[1].buttons)
            end
        elseif (contactStatus.currentStanding == 6) then -- test bit mission
            if(id == 1) then -- cancel misison
                local task = Task.new() -- empty task
                Player.AddUpdateTask(task);
                contactStatus.currentStanding = 4;
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[6].message, contactsForZone.LuaBot.contactDialogs[6].buttons)
            elseif (id == 2) then -- show mapmenu
                MapClientSession.MapMenu(client);
            elseif (id == 3) then -- leave
                --Server handles
            elseif (id == 4) then -- deliver bit
                contactStatus.currentStanding = 7;
                testTask.isComplete = true;
                testTask.location.mapName = "Atlas Park"; -- set return location
                testTask.state = "Return to LuaBot";
                Player.AddUpdateTask(testTask);
                MapClientSession.SendFloatingInfo(8)
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[10].message, contactsForZone.LuaBot.contactDialogs[10].buttons)
            end
        elseif (contactStatus.currentStanding == 7) then -- test bit mission complete
            if(id == 1) then -- cancel misison
                local task = Task.new() -- empty task
                Player.AddUpdateTask(task);
                contactStatus.currentStanding = 0;
                MapClientSession.Contact_dialog(contactsForZone.LuaBot.contactDialogs[1].message, contactsForZone.LuaBot.contactDialogs[1].buttons)
            elseif (id == 2) then -- Mapmenu
                MapClientSession.MapMenu(client);
            elseif (id == 3) then -- leave
                --Server handles
            end
        end

        Player.AddUpdateContact(contactStatus);
    else
        printDebug("ContactStatus false");
    end
end

LuaBot.CreateContact = function()
    
    local contact = Contact.new()
    contact.name = "LuaBot"
    contact.currentStanding = 0
    contact.notifyPlayer = true
    contact.npcId = 1144
    contact.hasLocation = true
    contact.taskIndex = 0
    contact.locationDescription = "Outbreak"
    contact.location = Destination.new()
    contact.location.location = vec3.new(-90, 0, 170)
    contact.location.name = "Outbreak"
    contact.location.mapName =  "Outbreak"
    contact.confidantThreshold = 2
    contact.friendThreshold = 4
    contact.completeThreshold = 6
    contact.canUseCell = false

    return contact
end

LuaBot.CreateTestContact = function()
    local contact = Contact.new()
    contact.name = "LuaBot Test"
    contact.currentStanding = 0
    contact.notifyPlayer = true
    contact.npcId = 1143
    contact.hasLocation = true
    contact.taskIndex = 1
    contact.locationDescription = "Outbreak"
    contact.location = Destination.new()
    contact.location.location = vec3.new(-90, 0, 170)
    contact.location.name = "Outbreak"
    contact.location.mapName =  "Outbreak"
    contact.confidantThreshold = 3
    contact.friendThreshold = 5
    contact.completeThreshold = 7
    contact.canUseCell = false
    
    return contact
end

LuaBot.CreateTask = function()
    local task = Task.new()
    task.dbId = 1
    task.taskIdx = 0 -- Match contact index to show on correct contact in contact list
    task.description = "Talk with LuaBot"
    task.owner = "LuaBot"
    task.detail = "LuaBot Task testing detail........<br><br>Accessing.......<br><br>Accessing.......<br><br>Accessing.......Data not found"
    task.state = ""
    task.inProgressMaybe = true
    task.isComplete = false
    task.isAbandoned = false
    task.finishTime = 0
    task.unknownInt1 = 1
    task.unknownInt2 = 1
    task.hasLocation = true
    task.boardTrain = false
    task.location = Destination.new()
    task.location.location = vec3.new(20, 0, 586)
    task.location.name  = "Outbreak"
    task.location.mapName = "Outbreak"

    return task
    
end
 
LuaBot.CreateAtlasTask = function()
    local task = Task.new()
    task.dbId = 1 -- 1939 - 200
    task.taskIdx = 0
    task.description = "Deliver Bit to LuaBot in Atlas Park"
    task.owner = "LuaBot"
    task.detail = "LuaBot in Outbreak requests you to deliver a test bit to LuaBot in Atlas Park. Use the /mapmenu or the mapmenu in LuaBot's contact dialog to travel."
    task.state = ""
    task.inProgressMaybe = true
    task.isComplete = false
    task.isAbandoned = false
    task.finishTime = 0
    task.unknownInt1 = 1
    task.unknownInt2 = 1
    task.hasLocation = true
    task.boardTrain = true
    task.location = Destination.new()
    task.location.location = vec3.new(112, 16, -216)
    task.location.name  = "City_01_01"
    task.location.mapName = "Atlas Park"

    return task
end

-- *********************************CONTACT DIALOGS********************************

    LuaBot.contactDialogs[2] = {
        message = [[<img src="npc:1144" align="left"><b>Player Stats</b><br><br>Choose an item below to adjust the corresponding player stat.<br>
        Don't worry these will not kill you.....permanently.<br>]],
        buttons = {
            button01 = {"Set Hp to 1","CONTACTLINK_HELLO"},
            button02 = {"Set Hp to max","CONTACTLINK_MAIN"},
            button03 = {"Set End to 1","CONTACTLINK_DONTKNOW"},
            button04 = {"Set End to Max","CONTACTLINK_MISSIONS"} ,
            button05 = {"Give 1000 XP","CONTACTLINK_LONGMISSION"} ,
            button06 = {"Give 1000 Debt","CONTACTLINK_SHORTMISSION"} ,
            button07 = {"Give 1000 Inf","CONTACTLINK_ACCEPTLONG"} ,
            button08 = {"Level up/Train","CONTACTLINK_TRAIN"} ,
            button09 = {"Reset Stats","CONTACTLINK_INTRODUCE"} ,
            button10 = {"More","CONTACTLINK_NOTLEADER"} ,
            button11 = {"Back","CONTACTLINK_WRONGMODE"} 
        }
    } 
    
    LuaBot.contactDialogs[3] = {
        message = [[<img src="npc:1144" align="left"><b>Player Stats Page 2</b><br><br>Choose an item below to adjust the corresponding player stat.<br>
                    Don't worry these will not kill you.....permanently.<br>]],
        buttons = {
            button01 = {"Give Random Inspiration","CONTACTLINK_HELLO"},
            button02 = {"Give Random Enhancement","CONTACTLINK_MAIN"},
            button03 = {"Set Title","CONTACTLINK_CHOOSE_TITLE"},
            button04 = {"Give Random Temp Power","CONTACTLINK_LONGMISSION"} ,
            --button05 = {"Give 1000 XP","CONTACTLINK_LONGMISSION"} ,
            --button06 = {"Give 1000 Debt","CONTACTLINK_SHORTMISSION"} ,
            --button07 = {"Give 1000 Inf","CONTACTLINK_ACCEPTLONG"} ,
            --button08 = {"Give Random Inspiration","CONTACTLINK_ACCEPTSHORT"} ,
            --button09 = {"Give Random Enhancement","CONTACTLINK_INTRODUCE"} ,
            --button10 = {"Reset Stats","CONTACTLINK_NOTLEADER"} ,
            button11 = {"Back","CONTACTLINK_WRONGMODE"} 
        }
    } 
        
    LuaBot.contactDialogs[4] = {
        message = [[<img src="npc:1144" align="left"><b>Contacts</b><br><br>Choose an item below to test contacts.<br><br>
                    <color #2189b9>Open the contact list to see the changes.</color>]],
        buttons = {
            button01 = {"Add LuaBot Test Contact","CONTACTLINK_HELLO"},
            button02 = {"Increase standing","CONTACTLINK_MAIN"},
            button03 = {"Lower standing","CONTACTLINK_MISSIONS"}, 
            button04 = {"Add Call button","CONTACTLINK_LONGMISSION"} ,
            button05 = {"Remove Call button","CONTACTLINK_SHORTMISSION"} ,
            button06 = {"Remove Contact","CONTACTLINK_ACCEPTLONG"} ,
            button07 = {"Back","CONTACTLINK_INTRODUCE"}
        }
    } 
    
    LuaBot.contactDialogs[6] = {
        message = [[<img src="npc:1144" align="left"><b>Tasks</b><br><br>Choose an item below to test tasks.<br><br>
                    Tasks require a contact to be listed. I'll add one for you.<br><br>
                    <color #2189b9>Open the tasks list to see some of the changes.</color>]],
        buttons = {
            button01 = {"Add Task","CONTACTLINK_HELLO"},
            button02 = {"Select Task","CONTACTLINK_MAIN"},
            button03 = {"Update Task Name","CONTACTLINK_MISSIONS"}, 
            button04 = {"Set Task to another zone","CONTACTLINK_LONGMISSION"} , -- Test mission complete sounds?
            button05 = {"Complete Task","CONTACTLINK_SHORTMISSION"} ,
            button06 = {"Remove Task","CONTACTLINK_ACCEPTLONG"} ,
            button07 = {"Task to Atlas Park","CONTACTLINK_ACCEPTSHORT"},
            button08 = {"Back","CONTACTLINK_INTRODUCE"}
        }
    } 
    
    LuaBot.contactDialogs[7] = {
        message = [[<img src="npc:1144" align="left"><b>Clues/Souvenirs</b><br><br>Choose an item below to test Clues and Souvenirs.<br><br>
                    <color #2189b9>Open the Clue/Souvenir list to see the changes.</color>]],
        buttons = {
            button01 = {"Add Clue","CONTACTLINK_HELLO"},
            button02 = {"Add Souvenir","CONTACTLINK_MAIN"},
            button03 = {"Remove Clue","CONTACTLINK_MISSIONS"}, 
            button04 = {"Remove Souvenir","CONTACTLINK_LONGMISSION"}, 
            --button05 = {"Complete Task","CONTACTLINK_SHORTMISSION"} ,
            --button06 = {"Remove Task","CONTACTLINK_ACCEPTLONG"} ,
            button07 = {"",""},
            button08 = {"Back","CONTACTLINK_INTRODUCE"}
        }
    } 

    LuaBot.contactDialogs[8] = {
        message = [[<img src="npc:1144" align="left"><br>Thank you for accepting this test task.<br>
                    Please deliver the bit to my counter part in Atlas Park.<br><br>
                    <color #2189b9>You can use the mapmanu slash command or the mapmenu option in the list.</color>]],
        buttons = {
            button01 = {"Cancel Mission","CONTACTLINK_HELLO"},
            button02 = {"Mapmenu","CONTACTLINK_MAIN"},
            --button03 = {"Update Task Name","CONTACTLINK_MISSIONS"}, 
            --button04 = {"Set Task to another zone","CONTACTLINK_LONGMISSION"} 
            --button05 = {"Complete Task","CONTACTLINK_SHORTMISSION"} ,
            --button06 = {"Remove Task","CONTACTLINK_ACCEPTLONG"} ,
            button07 = {"",""},
            button08 = {"Leave","CONTACTLINK_BYE"}
        }
    }



    LuaBot.contactDialogs[10] = {
        message = [[<img src="npc:1144" align="left"><br>Thank you for delivering this bit.<br>
                    Return to my counter part to finish this test.<br><br>
                    <color #2189b9>You can use the mapmanu slash command or the mapmenu option in the list.</color>]],
        buttons = {
            button01 = {"Mapmenu","CONTACTLINK_MAIN"},
            --button02 = {"Mapmenu","CONTACTLINK_MAIN"},
            --button03 = {"Update Task Name","CONTACTLINK_MISSIONS"}, 
            --button04 = {"Set Task to another zone","CONTACTLINK_LONGMISSION"} 
            --button05 = {"Complete Task","CONTACTLINK_SHORTMISSION"} ,
            --button06 = {"Remove Task","CONTACTLINK_ACCEPTLONG"} ,
            button07 = {"",""},
            button08 = {"Leave","CONTACTLINK_BYE"}
        }
    }

    LuaBot.contactDialogs[11] = {
        message = [[<img src="npc:1144" align="left"><br>This concludes the mission test.<br>
                    Thank you for your participation.<br><br>]],
        buttons = {
            button01 = {"Main Page","CONTACTLINK_HELLO"},
            --button02 = {"Mapmenu","CONTACTLINK_MAIN"},
            --button03 = {"Update Task Name","CONTACTLINK_MISSIONS"}, 
            --button04 = {"Set Task to another zone","CONTACTLINK_LONGMISSION"} 
            --button05 = {"Complete Task","CONTACTLINK_SHORTMISSION"} ,
            --button06 = {"Remove Task","CONTACTLINK_ACCEPTLONG"} ,
            button07 = {"",""},
            button08 = {"Leave","CONTACTLINK_BYE"}
        }
    }



    -- Must be at end
if(contactsForZone ~= nil) then
    contactsForZone.LuaBot = LuaBot
end
printDebug('luaBot script loaded')