-- OUTBREAK
printDebug('luaBot script loading...');

local LuaBot = {};
LuaBot.spawned = false;
LuaBot.name = 'LuaBot';
LuaBot.model = 'Jumpbot_02';
LuaBot.location = {};
LuaBot.location.coordinates = vec3.new(-90, 0, 170);
LuaBot.location.orientation = vec3.new(0, 0, 0);
LuaBot.variation = 1;
LuaBot.expected = false;
LuaBot.entityId = nil;
LuaBot.minLevel = 0; -- Level requirement?

LuaBot.isStore = true;
LuaBot.storeNames = "All_SL9_CL2,FBSA";
LuaBot.storeInventoryCount = 100;
LuaBot.SetupStore = function()
    MapClientSession.SetNpcStore(contactsForZone.LuaBot.entityId, contactsForZone.LuaBot.storeNames, contactsForZone.LuaBot.storeInventoryCount);
end

LuaBot.contactDialogs = {};
LuaBot.contacts = {};
LuaBot.tasks = {};

LuaBot.clue = Clue.new();
LuaBot.clue.name = "LuaBot Test Clue";
LuaBot.clue.displayName = "LuaBot Clue";
LuaBot.clue.detail = "Test clue from LuaBot";
LuaBot.clue.iconFile = "icon"; -- Client file name?

LuaBot.souvenir = Souvenir.new();
LuaBot.souvenir.name = "LuaBot souvenir";
LuaBot.souvenir.description = "A Souvenir from LuaBot";
LuaBot.souvenir.icon = "icon"; -- unknown?

LuaBot.dialogPages = {};

LuaBot.createContactDialogsWithHeroName = function(heroName)
    contactsForZone.LuaBot.dialogPages[1] = {};
    contactsForZone.LuaBot.dialogPages[1].contactDialog = { 
        message = string.format([[<img src="npc:1144" align="left">Hello, {HeroName}.....bzzt.....I mean %s. I am LuaBot. Here to assist you in testing the
                Lua scripting interface.<br><br>Please select an option below to test.]], heroName),
        buttons =  {
            button1 = {"Player Stats","CONTACTLINK_HELLO"},
            button2 = {"Contacts","CONTACTLINK_MAIN"},
            button3 = {"Tasks","CONTACTLINK_MISSIONS"}, 
            button4 = {"MapMenu","CONTACTLINK_LONGMISSION"}, 
            button5 = {"Clues","CONTACTLINK_SHORTMISSION"}, 
            button6 = {"Store","CONTACTLINK_ACCEPTLONG"},
            button7 = {"Logging","CONTACTLINK_ACCEPTSHORT"},
            button8 = {"",""},
            button9 = {"Leave","CONTACTLINK_BYE"}
        }
    }
    
    contactsForZone.LuaBot.dialogPages[1].actions = {};
    contactsForZone.LuaBot.dialogPages[1].actions["CONTACTLINK_HELLO"] = function()
        printDebug("LuaBot moving to stat mode...");
        local contactStatus = Contacts.FindContactByName("LuaBot");
        if(contactStatus ~= false) then
            contactStatus.dialogScreenIdx = 2;
            Player.AddUpdateContact(contactStatus);
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[2].contactDialog.message, contactsForZone.LuaBot.dialogPages[2].contactDialog.buttons);
        end

        local LuaBotMission = Tasks.FindTaskByTaskIdx(0);
        if(LuaBotMission ~= false) then
            local task = Task.new(); -- empty task
            Player.AddUpdateTask(task);
        end
    end

    contactsForZone.LuaBot.dialogPages[1].actions["CONTACTLINK_MAIN"] = function()
        printDebug("LuaBot moving to contact mode...")
        local contactStatus = Contacts.FindContactByName("LuaBot");
        if(contactStatus ~= false) then
            contactStatus.dialogScreenIdx = 4;
            Player.AddUpdateContact(contactStatus);
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[4].contactDialog.message, contactsForZone.LuaBot.dialogPages[4].contactDialog.buttons);
        end
    end

    contactsForZone.LuaBot.dialogPages[1].actions["CONTACTLINK_MISSIONS"] = function()
        printDebug("LuaBot moving to task mode...");
        local contactStatus = Contacts.FindContactByName("LuaBot");
        if(contactStatus ~= false) then
            contactStatus.dialogScreenIdx = 5; -- Tasks
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[5].contactDialog.message, contactsForZone.LuaBot.dialogPages[5].contactDialog.buttons);
            Player.AddUpdateContact(contactStatus);
        end
    end

    contactsForZone.LuaBot.dialogPages[1].actions["CONTACTLINK_LONGMISSION"] = function()
        MapClientSession.MapMenu(client);
    end

    contactsForZone.LuaBot.dialogPages[1].actions["CONTACTLINK_SHORTMISSION"] = function()
        printDebug("LuaBot moving to clue/souvenir mode...")
        local contactStatus = Contacts.FindContactByName("LuaBot");
        if(contactStatus ~= false) then
            contactStatus.dialogScreenIdx = 6; -- Clues/Souvenirs
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[6].contactDialog.message, contactsForZone.LuaBot.dialogPages[6].contactDialog.buttons);
            Player.AddUpdateContact(contactStatus);
        end
    end

    contactsForZone.LuaBot.dialogPages[1].actions["CONTACTLINK_ACCEPTLONG"] = function()
        MapClientSession.OpenStore(contactsForZone.LuaBot.entityId);
    end

    contactsForZone.LuaBot.dialogPages[1].actions["CONTACTLINK_ACCEPTSHORT"] = function()
        local contactStatus = Contacts.FindContactByName("LuaBot");
        if(contactStatus ~= false) then
            contactStatus.dialogScreenIdx = 11; -- Logging
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[11].contactDialog.message, contactsForZone.LuaBot.dialogPages[11].contactDialog.buttons);
            Player.AddUpdateContact(contactStatus);
        end
    end

    contactsForZone.LuaBot.dialogPages[10] = {};
    contactsForZone.LuaBot.dialogPages[10].contactDialog = {
        message = string.format([[<img src="npc:1144" align="left"><br>Ah %s, I see you have the test bit.<br>
                    Please hand the bit to me now.<br>]], heroName),
        buttons = {
            button01 = {"Deliver Bit","CONTACTLINK_HELLO"},
            button02 = {"",""},
            button03 = {"Leave","CONTACTLINK_BYE"}
        }
    }

    contactsForZone.LuaBot.dialogPages[10].actions = {};
    contactsForZone.LuaBot.dialogPages[10].actions["CONTACTLINK_HELLO"] = function()
        local testTask = Tasks.FindTaskByTaskIdx(0);
        local contactStatus = Contacts.FindContactByName("LuaBot");
        if(testTask ~= false and contactStatus ~= false) then
            testTask.isComplete = true;
            testTask.location.mapName = "Atlas Park"; -- set return location
            testTask.state = "Return to LuaBot";
            Player.AddUpdateTask(testTask);

            contactStatus.currentStanding = 2;
            contactStatus.dialogScreenIdx = 8;
            Player.AddUpdateContact(contactStatus);

            MapClientSession.SendFloatingInfo(8);
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[8].contactDialog.message, contactsForZone.LuaBot.dialogPages[8].contactDialog.buttons);
        end
    end

end

LuaBot.startDialogs = function()
    Player.SetActiveDialogCallback(contactsForZone.LuaBot.callback);
    local contactStatus = Contacts.FindContactByName("LuaBot");
    if(contactStatus ~= false) then
        local testTask = Tasks.FindTaskByTaskIdx(0);

        if (contactStatus.currentStanding == 1 and testTask ~= false and contactStatus.dialogScreenIdx == 0) then -- Give Bit
            contactStatus.dialogScreenIdx = 10;
            Player.AddUpdateContact(contactStatus);
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[10].contactDialog.message, contactsForZone.LuaBot.dialogPages[10].contactDialog.buttons);
        elseif(contactStatus.currentStanding == 2 and contactStatus.dialogScreenIdx == 8) then -- Return to LuaBot in atlas
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[8].contactDialog.message, contactsForZone.LuaBot.dialogPages[8].contactDialog.buttons);
        elseif(contactStatus.currentStanding == 2 and testTask.location.mapName == "Outbreak") then -- Finished mission
            contactStatus.dialogScreenIdx = 9;
            Player.AddUpdateContact(contactStatus);
            
            testTask = Task.new();
            Player.AddUpdateTask(testTask);
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[9].contactDialog.message, contactsForZone.LuaBot.dialogPages[9].contactDialog.buttons);
        elseif(contactStatus.currentStanding == 1 and testTask.location.mapName == "Atlas Park") then -- On mission to Atlas
            contactStatus.dialogScreenIdx = 7;
            Player.AddUpdateContact(contactStatus);
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[7].contactDialog.message, contactsForZone.LuaBot.dialogPages[7].contactDialog.buttons);
        else
            contactStatus.locationDescription = "Outbreak";
            contactStatus.location = Destination.new();
            contactStatus.location.location = vec3.new(-90, 0, 170);
            contactStatus.location.name = "Outbreak";
            contactStatus.location.mapName = "Outbreak";
            contactStatus.dialogScreenIdx = 1;
            Player.AddUpdateContact(contactStatus);
            MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[1].contactDialog.message, contactsForZone.LuaBot.dialogPages[1].contactDialog.buttons);
        end
    else
        contactStatus = contactsForZone.LuaBot.CreateContact();
        Player.AddUpdateContact(contactStatus);
        MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[1].contactDialog.message, contactsForZone.LuaBot.dialogPages[1].contactDialog.buttons);
    end
end

LuaBot.callback = function(id)
    printDebug("LuaBot Callback: " .. tostring(id));
    local contactStatus = Contacts.FindContactByName("LuaBot");
    if (contactStatus ~= false) then
        local button = Contacts.ParseContactButton(id);
        printDebug("LuaBot.Callback Button String: " .. tostring(button));

        if(button ~= false and button ~= "CONTACTLINK_BYE") then
            if(contactStatus.settingTitle ~= true) then
                contactsForZone.LuaBot.dialogPages[contactStatus.dialogScreenIdx].actions[button]();
            else
                contactStatus.settingTitle = false;
                Player.AddUpdateContact(contactStatus);
            end
        elseif (button == "CONTACTLINK_BYE") then
            printDebug("LuaBot Callback: Bye button");
        else
            printDebug("LuaBot Callback: Button not found");
        end
    end
end

LuaBot.CreateContact = function()
    
    local contact = Contact.new();
    contact.name = "LuaBot";
    contact.currentStanding = 0;
    contact.notifyPlayer = true;
    contact.npcId = 1144;
    contact.hasLocation = true;
    contact.taskIndex = 0;
    contact.locationDescription = "Outbreak";
    contact.location = Destination.new();
    contact.location.location = vec3.new(-90, 0, 170);
    contact.location.name = "Outbreak";
    contact.location.mapName =  "Outbreak";
    contact.confidantThreshold = 2;
    contact.friendThreshold = 4;
    contact.completeThreshold = 6;
    contact.canUseCell = false;
    contact.dialogScreenIdx = 1;

    return contact;
end

LuaBot.CreateTestContact = function()
    local contact = Contact.new();
    contact.name = "LuaBot Test";
    contact.currentStanding = 0;
    contact.notifyPlayer = true;
    contact.npcId = 1143;
    contact.hasLocation = true;
    contact.taskIndex = 1;
    contact.locationDescription = "Outbreak";
    contact.location = Destination.new();
    contact.location.location = vec3.new(-90, 0, 170);
    contact.location.name = "Outbreak";
    contact.location.mapName =  "Outbreak";
    contact.confidantThreshold = 3;
    contact.friendThreshold = 5;
    contact.completeThreshold = 7;
    contact.canUseCell = false;
    
    return contact;
end

LuaBot.CreateTask = function()
    local task = Task.new();
    task.dbId = 1;
    task.taskIdx = 0; -- Match contact index to show on correct contact in contact list
    task.description = "Talk with LuaBot";
    task.owner = "LuaBot";
    task.detail = "LuaBot Task testing detail........<br><br>Accessing.......<br><br>Accessing.......<br><br>Accessing.......Data not found";
    task.state = "";
    task.inProgressMaybe = true;
    task.isComplete = false;
    task.isAbandoned = false;
    task.finishTime = 0;
    task.unknownInt1 = 1;
    task.unknownInt2 = 1;
    task.hasLocation = true;
    task.boardTrain = false;
    task.location = Destination.new();
    task.location.location = vec3.new(20, 0, 586);
    task.location.name  = "Outbreak";
    task.location.mapName = "Outbreak";

    return task;
    
end
 
LuaBot.CreateAtlasTask = function()
    local task = Task.new()
    task.dbId = 1;
    task.taskIdx = 0;
    task.description = "Deliver Bit to LuaBot in Atlas Park";
    task.owner = "LuaBot";
    task.detail = "LuaBot in Outbreak requests you to deliver a test bit to LuaBot in Atlas Park. Use the /mapmenu or the mapmenu in LuaBot's contact dialog to travel.";
    task.state = "";
    task.inProgressMaybe = true;
    task.isComplete = false;
    task.isAbandoned = false;
    task.finishTime = 0;
    task.unknownInt1 = 1;
    task.unknownInt2 = 1;
    task.hasLocation = true;
    task.boardTrain = true;
    task.location = Destination.new();
    task.location.location = vec3.new(112, 16, -216);
    task.location.name  = "City_01_01";
    task.location.mapName = "Atlas Park";

    return task;
end

-- *********************************CONTACT DIALOGS********************************
LuaBot.dialogPages[2] = {};
LuaBot.dialogPages[2].contactDialog = {
    message = [[<img src="npc:1144" align="left"><b>Player Stats</b><br><br>Choose an item below to adjust the corresponding player stat.<br>
    Don't worry these will not kill you.....permanently.<br>]],
    buttons = {
        button01 = {"Set Hp to 1","CONTACTLINK_HELLO"},
        button02 = {"Set Hp to max","CONTACTLINK_MAIN"},
        button03 = {"Set End to 1","CONTACTLINK_MISSIONS"}, 
        button04 = {"Set End to Max","CONTACTLINK_LONGMISSION"} ,
        button05 = {"Give 1000 XP","CONTACTLINK_SHORTMISSION"} ,
        button06 = {"Give 1000 Debt","CONTACTLINK_ACCEPTLONG"} ,
        button07 = {"Give 1000 Inf","CONTACTLINK_ACCEPTSHORT"} ,
        button08 = {"Level up/Train","CONTACTLINK_INTRODUCE"} ,
        button09 = {"Reset Stats","CONTACTLINK_INTRODUCE_CONTACT1"} ,
        button10 = {"More","CONTACTLINK_INTRODUCE_CONTACT2"} ,
        button11 = {"Back","CONTACTLINK_ACCEPT_CONTACT2"} 
    }
} 

LuaBot.dialogPages[2].actions = {};
LuaBot.dialogPages[2].actions["CONTACTLINK_HELLO"] = function()
    Player.SetHp(1);
end

LuaBot.dialogPages[2].actions["CONTACTLINK_MAIN"] = function()
    Player.GiveHp(99999); -- Should be max
end

LuaBot.dialogPages[2].actions["CONTACTLINK_MISSIONS"] = function ()
    Player.SetEnd(1);
end

LuaBot.dialogPages[2].actions["CONTACTLINK_LONGMISSION"] = function ()
    Player.GiveEnd(100);
end 

LuaBot.dialogPages[2].actions["CONTACTLINK_SHORTMISSION"] = function ()
    Player.GiveXp(1000);
end 

LuaBot.dialogPages[2].actions["CONTACTLINK_ACCEPTLONG"] = function ()
    Player.GiveDebt(1000);
end 

LuaBot.dialogPages[2].actions["CONTACTLINK_ACCEPTSHORT"] = function ()
    Player.GiveInf(1000);
end 

LuaBot.dialogPages[2].actions["CONTACTLINK_INTRODUCE"] = function ()
     --Train/Level
     Player.LevelUp();
end 

LuaBot.dialogPages[2].actions["CONTACTLINK_INTRODUCE_CONTACT1"] = function ()
    --reset stats
    Player.GiveHp(99999);
    Player.GiveEnd(100);
    Player.SetXp(0);
    Player.SetDebt(0);
    Player.SetInf(0);
end 

LuaBot.dialogPages[2].actions["CONTACTLINK_INTRODUCE_CONTACT2"] = function ()
     --More 
     local contactStatus = Contacts.FindContactByName("LuaBot");
     if(contactStatus ~= false) then
         contactStatus.dialogScreenIdx = 3;
         Player.AddUpdateContact(contactStatus);
         MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[3].contactDialog.message, contactsForZone.LuaBot.dialogPages[3].contactDialog.buttons);
     end
end 

LuaBot.dialogPages[2].actions["CONTACTLINK_ACCEPT_CONTACT2"] = function ()
   -- Back
   local contactStatus = Contacts.FindContactByName("LuaBot");
   if(contactStatus ~= false) then
       contactStatus.dialogScreenIdx = 1;
       Player.AddUpdateContact(contactStatus);
       MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[1].contactDialog.message, contactsForZone.LuaBot.dialogPages[1].contactDialog.buttons);
   end
end 

LuaBot.dialogPages[3] = {};
LuaBot.dialogPages[3].contactDialog = {
    message = [[<img src="npc:1144" align="left"><b>Player Stats Page 2</b><br><br>Choose an item below to adjust the corresponding player stat.<br>
                Don't worry these will not kill you.....permanently.<br>]],
    buttons = {
        button01 = {"Give Random Inspiration","CONTACTLINK_HELLO"},
        button02 = {"Give Random Enhancement","CONTACTLINK_MAIN"},
        button03 = {"Set Title","CONTACTLINK_MISSIONS"},
        button04 = {"Give Random Temp Power","CONTACTLINK_LONGMISSION"},
        button05 = {"", ""},
        button06 = {"Back","CONTACTLINK_SHORTMISSION"} 
    }
}

LuaBot.dialogPages[3].actions = {};
LuaBot.dialogPages[3].actions["CONTACTLINK_HELLO"] = function ()
    Player.GiveRandomInsp();
end

LuaBot.dialogPages[3].actions["CONTACTLINK_MAIN"] = function ()
    local level = Player.GetLevel();
    Player.GiveRandomEnhancement(level);
end

LuaBot.dialogPages[3].actions["CONTACTLINK_MISSIONS"] = function ()
    local contactStatus = Contacts.FindContactByName("LuaBot");
    if (contactStatus ~= false) then
        contactStatus.settingTitle = true;
        Player.AddUpdateContact(contactStatus);
        Player.OpenTitleMenu();
        MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[3].contactDialog.message, contactsForZone.LuaBot.dialogPages[3].contactDialog.buttons);
    end
end

LuaBot.dialogPages[3].actions["CONTACTLINK_LONGMISSION"] = function ()
    Player.GiveRandomTempPower();
end

LuaBot.dialogPages[3].actions["CONTACTLINK_SHORTMISSION"] = function ()
    -- Back
    local contactStatus = Contacts.FindContactByName("LuaBot");
    if(contactStatus ~= false) then
        contactStatus.dialogScreenIdx = 2;
        contactsForZone.LuaBot.settingTitle = true;
        Player.AddUpdateContact(contactStatus);
        MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[2].contactDialog.message, contactsForZone.LuaBot.dialogPages[2].contactDialog.buttons);
    end
end

LuaBot.dialogPages[4] = {};
LuaBot.dialogPages[4].contactDialog = {
    message = [[<img src="npc:1144" align="left"><b>Contacts</b><br><br>Choose an item below to test contacts.<br><br>
                <color #2189b9>Open the contact list to see the changes.</color>]],
    buttons = {
        button01 = {"Add LuaBot Test Contact","CONTACTLINK_HELLO"},
        button02 = {"Increase standing","CONTACTLINK_MAIN"},
        button03 = {"Lower standing","CONTACTLINK_MISSIONS"}, 
        button04 = {"Add Call button","CONTACTLINK_LONGMISSION"} ,
        button05 = {"Remove Call button","CONTACTLINK_SHORTMISSION"} ,
        button06 = {"Remove Contact","CONTACTLINK_ACCEPTLONG"} ,
        button07 = {"Back","CONTACTLINK_ACCEPTSHORT"}
    }
} 

LuaBot.dialogPages[4].actions = {};
LuaBot.dialogPages[4].actions["CONTACTLINK_HELLO"] = function ()
    local testContactStatus = contactsForZone.LuaBot.CreateTestContact();
    Player.AddUpdateContact(testContactStatus);
end

LuaBot.dialogPages[4].actions["CONTACTLINK_MAIN"] = function ()
    local testContactStatus = Contacts.FindContactByName("LuaBot Test");
    if(testContactStatus ~= false and testContactStatus.currentStanding < 7) then
        testContactStatus.currentStanding = testContactStatus.currentStanding + 1;
        Player.AddUpdateContact(testContactStatus);
    end
end

LuaBot.dialogPages[4].actions["CONTACTLINK_MISSIONS"] = function ()
    local testContactStatus = Contacts.FindContactByName("LuaBot Test");
    if(testContactStatus ~= false and testContactStatus.currentStanding > 0) then
        testContactStatus.currentStanding = testContactStatus.currentStanding - 1;
        Player.AddUpdateContact(testContactStatus);
    end
end

LuaBot.dialogPages[4].actions["CONTACTLINK_LONGMISSION"] = function ()
    local testContactStatus = Contacts.FindContactByName("LuaBot Test");
    if(testContactStatus ~= false) then
        testContactStatus.canUseCell = true;
        Player.AddUpdateContact(testContactStatus);
    end
end

LuaBot.dialogPages[4].actions["CONTACTLINK_SHORTMISSION"] = function ()
    local testContactStatus = Contacts.FindContactByName("LuaBot Test");
    if(testContactStatus ~= false) then
        testContactStatus.canUseCell = false;
        Player.AddUpdateContact(testContactStatus);
    end
end

LuaBot.dialogPages[4].actions["CONTACTLINK_ACCEPTLONG"] = function ()
     --reset/Remove contact
     local testContactStatus = Contacts.FindContactByName("LuaBot Test");
     if(testContactStatus ~= false) then
        Player.RemoveContact(testContactStatus);
    end 
end

LuaBot.dialogPages[4].actions["CONTACTLINK_ACCEPTSHORT"] = function ()
    local contactStatus = Contacts.FindContactByName("LuaBot");
    if(contactStatus ~= false) then
        contactStatus.dialogScreenIdx = 1;
        Player.AddUpdateContact(contactStatus);
        MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[1].contactDialog.message, contactsForZone.LuaBot.dialogPages[1].contactDialog.buttons);
    end
end

LuaBot.dialogPages[5] = {};
LuaBot.dialogPages[5].contactDialog = {
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

LuaBot.dialogPages[5].actions = {};
LuaBot.dialogPages[5].actions["CONTACTLINK_HELLO"] = function()
    local task = contactsForZone.LuaBot.CreateTask();
    Player.AddUpdateTask(task);
end

LuaBot.dialogPages[5].actions["CONTACTLINK_MAIN"] = function()
    local testTask = Tasks.FindTaskByTaskIdx(0);
    if(testTask ~= false) then
        Player.SelectTask(testTask);
    end
end

LuaBot.dialogPages[5].actions["CONTACTLINK_MISSIONS"] = function()
    local testTask = Tasks.FindTaskByTaskIdx(0);
    if(testTask ~= false) then
        testTask.description = 'Task Updated - Talked to LuaBot';
        Player.AddUpdateTask(testTask);
    end
end

LuaBot.dialogPages[5].actions["CONTACTLINK_LONGMISSION"] = function()
    local testTask = Tasks.FindTaskByTaskIdx(0);
    if(testTask ~= false) then
        testTask.boardTrain = true;
        testTask.location.name  = "Atlas park";
        testTask.location.mapName = "Atlas park";
        Player.AddUpdateTask(testTask);
    end
end

LuaBot.dialogPages[5].actions["CONTACTLINK_SHORTMISSION"] = function()
    local testTask = Tasks.FindTaskByTaskIdx(0);
    if(testTask ~= false) then
        testTask.isComplete = true;
        testTask.state = 'Return to LuaBot';
        Player.AddUpdateTask(testTask);
    end
end

LuaBot.dialogPages[5].actions["CONTACTLINK_ACCEPTLONG"] = function()
    local task = Task.new(); -- empty task
    Player.AddUpdateTask(task);

    local contactStatus = Contacts.FindContactByName("LuaBot");
    if(contactStatus ~= false) then
        contactStatus.currentStanding = 0;
        Player.AddUpdateContact(contactStatus);
    end
end

LuaBot.dialogPages[5].actions["CONTACTLINK_ACCEPTSHORT"] = function()
      -- Task to LuaBot in Atlas
      local testTask = contactsForZone.LuaBot.CreateAtlasTask()
      Player.AddUpdateTask(testTask);
      
      local contactStatus = Contacts.FindContactByName("LuaBot");
      if(contactStatus ~= false) then
        contactStatus.dialogScreenIdx = 7;
        contactStatus.currentStanding = 1;
        Player.AddUpdateContact(contactStatus);
        MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[7].contactDialog.message, contactsForZone.LuaBot.dialogPages[7].contactDialog.buttons);
      end
end

LuaBot.dialogPages[5].actions["CONTACTLINK_INTRODUCE"] = function()
     --Back
     local contactStatus = Contacts.FindContactByName("LuaBot");
     if(contactStatus ~= false) then
        contactStatus.dialogScreenIdx = 1;
        Player.AddUpdateContact(contactStatus);
        MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[1].contactDialog.message, contactsForZone.LuaBot.dialogPages[1].contactDialog.buttons);
     end
end
    
LuaBot.dialogPages[6] = {};
LuaBot.dialogPages[6].contactDialog = {
    message = [[<img src="npc:1144" align="left"><b>Clues/Souvenirs</b><br><br>Choose an item below to test Clues and Souvenirs.<br><br>
                <color #2189b9>Open the Clue/Souvenir list to see the changes.</color>]],
    buttons = {
        button01 = {"Add Clue","CONTACTLINK_HELLO"},
        button02 = {"Add Souvenir","CONTACTLINK_MAIN"},
        button03 = {"Remove Clue","CONTACTLINK_MISSIONS"}, 
        button04 = {"Remove Souvenir","CONTACTLINK_LONGMISSION"}, 
        button05 = {"",""},
        button06 = {"Back","CONTACTLINK_INTRODUCE"}
    }
} 

LuaBot.dialogPages[6].actions = {};
LuaBot.dialogPages[6].actions["CONTACTLINK_HELLO"] = function()
    --add clue
    Player.AddClue(contactsForZone.LuaBot.clue);
    MapClientSession.SendFloatingInfo(5);
end

LuaBot.dialogPages[6].actions["CONTACTLINK_MAIN"] = function()
     --add souvenir
     Player.AddSouvenir(contactsForZone.LuaBot.souvenir);
     MapClientSession.SendFloatingInfo(7); -- Inspiration sound.
end

LuaBot.dialogPages[6].actions["CONTACTLINK_MISSIONS"] = function()
    Player.RemoveClue(contactsForZone.LuaBot.clue);
end

LuaBot.dialogPages[6].actions["CONTACTLINK_LONGMISSION"] = function()
    Player.RemoveSouvenir(contactsForZone.LuaBot.souvenir);
end

LuaBot.dialogPages[6].actions["CONTACTLINK_INTRODUCE"] = function()
    local contactStatus = Contacts.FindContactByName("LuaBot");
    if(contactStatus ~= false) then
       contactStatus.dialogScreenIdx = 1;
       Player.AddUpdateContact(contactStatus);
       MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[1].contactDialog.message, contactsForZone.LuaBot.dialogPages[1].contactDialog.buttons);
    end
end

LuaBot.dialogPages[7] = {};
LuaBot.dialogPages[7].contactDialog = {
    message = [[<img src="npc:1144" align="left"><br>Thank you for accepting this test task.<br>
                Please deliver the bit to my counter part in Atlas Park.<br><br>
                <color #2189b9>You can use the mapmanu slash command or the mapmenu option in the list.</color>]],
    buttons = {
        button01 = {"Cancel Mission","CONTACTLINK_HELLO"},
        button02 = {"Mapmenu","CONTACTLINK_MAIN"},
        button03 = {"",""},
        button04 = {"Leave","CONTACTLINK_BYE"}
    }
}

LuaBot.dialogPages[7].actions = {};
LuaBot.dialogPages[7].actions["CONTACTLINK_HELLO"] = function ()
    local task = Task.new() -- empty task
    Player.AddUpdateTask(task);

    local contactStatus = Contacts.FindContactByName("LuaBot");
    if(contactStatus ~= false) then
        contactStatus.dialogScreenIdx = 5;
        contactStatus.currentStanding = 0;
        Player.AddUpdateContact(contactStatus);
        MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[5].contactDialog.message, contactsForZone.LuaBot.dialogPages[5].contactDialog.buttons);
     end
end

LuaBot.dialogPages[7].actions["CONTACTLINK_MAIN"] = function ()
    MapClientSession.MapMenu(client);
end

LuaBot.dialogPages[8] = {};
LuaBot.dialogPages[8].contactDialog = {
    message = [[<img src="npc:1144" align="left"><br>Thank you for delivering this bit.<br>
                Return to my counter part to finish this test.<br><br>
                <color #2189b9>You can use the mapmanu slash command or the mapmenu option in the list.</color>]],
    buttons = {
        button01 = {"Mapmenu","CONTACTLINK_MAIN"},
        button02 = {"",""},
        button03 = {"Leave","CONTACTLINK_BYE"}
    }
}

LuaBot.dialogPages[8].actions = {};
LuaBot.dialogPages[8].actions["CONTACTLINK_MAIN"] = function()
    MapClientSession.MapMenu(client);
end

LuaBot.dialogPages[9] = {};
LuaBot.dialogPages[9].contactDialog = {
    message = [[<img src="npc:1144" align="left"><br>This concludes the mission test.<br>
                Thank you for your participation.<br><br>]],
    buttons = {
        button01 = {"Main Page","CONTACTLINK_HELLO"},
        button02 = {"",""},
        button03 = {"Leave","CONTACTLINK_BYE"}
    }
}

LuaBot.dialogPages[9].actions = {};
LuaBot.dialogPages[9].actions["CONTACTLINK_HELLO"] = function()
    local contactStatus = Contacts.FindContactByName("LuaBot");
    if(contactStatus ~= false) then
        contactStatus.dialogScreenIdx = 1;
        Player.AddUpdateContact(contactStatus);
        MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[1].contactDialog.message, contactsForZone.LuaBot.dialogPages[1].contactDialog.buttons);
     end
end


LuaBot.dialogPages[11] = {};
LuaBot.dialogPages[11].contactDialog = {
    message = [[<img src="npc:1144" align="left">Logging functions<br><br>
                Console Output is shown when the game is launch with -console<br>
                Console Print is shown in the ingame console (`)]],
    buttons = {
        button01 = {"Console Output","CONTACTLINK_HELLO"},
        button02 = {"Console Print","CONTACTLINK_MAIN"},
        button03 = {"",""},
        button04 = {"Back","CONTACTLINK_MISSIONS"}
    }
}

LuaBot.dialogPages[11].actions = {};
LuaBot.dialogPages[11].actions["CONTACTLINK_HELLO"] = function()
  MapClientSession.DeveloperConsoleOutput("Console Output test message from LuaBot");
end

LuaBot.dialogPages[11].actions["CONTACTLINK_MAIN"] = function()
    MapClientSession.ClientConsoleOutput("Console Print test message from LuaBot");
end

LuaBot.dialogPages[11].actions["CONTACTLINK_MISSIONS"] = function()
    local contactStatus = Contacts.FindContactByName("LuaBot");
    if(contactStatus ~= false) then
        contactStatus.dialogScreenIdx = 1;
        Player.AddUpdateContact(contactStatus);
        MapClientSession.Contact_dialog(contactsForZone.LuaBot.dialogPages[1].contactDialog.message, contactsForZone.LuaBot.dialogPages[1].contactDialog.buttons);
     end
end

    -- Must be at end
if(contactsForZone ~= nil) then
    contactsForZone.LuaBot = LuaBot;
end
printDebug('luaBot script loaded');