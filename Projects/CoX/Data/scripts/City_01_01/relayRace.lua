-- ATLAS PARK
printDebug('relayRace script loading...');

local RelayRace = {};
RelayRace.spawned = false;
RelayRace.name = "Sgt. Johnson";
-- Model_Bastion , Model_ColleenSaramago , Model_SergeantHicks , Model_SuzanneBernhard , Model_VicJohansson
RelayRace.model = "Model_VicJohansson"; -- 2009
RelayRace.location = {};
RelayRace.location.coordinates = vec3.new(192, 0, -117);
RelayRace.location.orientation = vec3.new(0,0,0);
RelayRace.variation = 1;
RelayRace.expected = false;
RelayRace.entityId = nil;
RelayRace.minLevel = 0;
RelayRace.isStore = false;
RelayRace.useMapInstance = true;
RelayRace.time = 0;

RelayRace.tasks = {};
RelayRace.tasks[1] = Task.new();
RelayRace.tasks[1].dbId = 0;
RelayRace.tasks[1].taskIdx = 0; -- Match contact index to show on correct contact in contact list
RelayRace.tasks[1].description = "Test police call box 1";
RelayRace.tasks[1].owner = "Sgt. Johnson";
RelayRace.tasks[1].detail = "Sgt. Johnson needs help testing the police call boxes around Atlas Park.<br>Head to the call box near the Sewer Network enterance.";
RelayRace.tasks[1].state = "";
RelayRace.tasks[1].inProgressMaybe = true;
RelayRace.tasks[1].isComplete = false;
RelayRace.tasks[1].isAbandoned = false;
RelayRace.tasks[1].finishTime = 30;
RelayRace.tasks[1].unknownInt1 = 1;
RelayRace.tasks[1].unknownInt2 = 1;
RelayRace.tasks[1].hasLocation = false;
RelayRace.tasks[1].boardTrain = false;
RelayRace.tasks[1].location = Destination.new();
RelayRace.tasks[1].location.location = vec3.new(461, 0, 778);
RelayRace.tasks[1].location.name  = "Atlas Park";
RelayRace.tasks[1].location.mapName = "Atlas Park";

RelayRace.locations = {};
RelayRace.locations[1] = {};
RelayRace.locations[1].name = "Patrol_Easy_1_City_01_01";
RelayRace.locations[1].coordinates = vec3.new(461, 3, 778);
RelayRace.locations[1].action = function()
    local contactStatus = Contacts.FindContactByName("Sgt. Johnson");
    
    if(contactStatus ~= false) then
        printDebug("RelayRace.locations[1].action: dialogScreenIdx: " .. tostring(contactStatus.dialogScreenIdx));
        if(contactStatus.dialogScreenIdx == 1) then
            contactStatus.currentStanding = 1;
            Player.AddUpdateContact(contactStatus);
        end
    end

    Player.SetActiveDialogCallback(contactsForZone.RelayRace.callback);
    
   
end

RelayRace.locations[2] = {};
RelayRace.locations[2].name = "Patrol_Easy_2_City_01_01";
RelayRace.locations[2].coordinates = vec3.new(-928, -15, 44);
RelayRace.locations[2].action = function()

end

RelayRace.locations[3] = {};
RelayRace.locations[3].name = "Patrol_Easy_3_City_01_01";
RelayRace.locations[3].coordinates = vec3.new(89, 0, -1785);
RelayRace.locations[4] = {};
RelayRace.locations[4].name = "Patrol_Easy_4_City_01_01";
RelayRace.locations[4].coordinates = vec3.new(679, 42, -1246);
RelayRace.locations[5] = {};
RelayRace.locations[5].name = "Patrol_Easy_5_City_01_01";
RelayRace.locations[5].coordinates = vec3.new(1099, 0, 124);


RelayRace.dialogPages = {};
RelayRace.dialogPages[2] = {};
RelayRace.dialogPages[2].contactDialog = {
    message = [[<img src="npc:2009" align="left"><br>Good! Now normally there would be 10 boxes 
    to check but since you have a speed..."issue" we'll just test the 5 nearby.<br><br>
    The first one is to the south near the enterance to the sewer network.
    Once you reach that box, "click" it. I'll record your time and let you know where the next box is.<br><br>
    Ready?<br><br><color #2189b9>Your time starts once you click "ready".</color>]],
    buttons = {
        button01 = {"Ready","CONTACTLINK_ACCEPTSHORT"},
        button02 = {"Nevermind","CONTACTLINK_WRONGMODE"}
    }
}

RelayRace.dialogPages[3] = {};
RelayRace.dialogPages[3].contactDialog = {
    message = [[<img src="npc:2009" align="left"><br>Well then I have nothing for you.]],
    buttons = {
        button01 = {"Leave","CONTACTLINK_BYE"}
    }
}

RelayRace.dialogPages[4] = {};
RelayRace.dialogPages[4].contactDialog = {
    message = [[<img src="npc:2009" align="left"><br>Hurry to the police call box!]],
    buttons = {
        button01 = {"Leave","CONTACTLINK_BYE"},
        button02 = {"Reset","CONTACTLINK_INTRODUCE_CONTACT1"},
    }
}

RelayRace.createContactDialogsWithHeroName = function (heroName)
    contactsForZone.RelayRace.dialogPages[1] = {};
    contactsForZone.RelayRace.dialogPages[1].contactDialog = {
        message = string.format([[<img src="npc:2009" align="left"><br>%s! Get over here!<br><br>
        We've received word that vilians will be returning to the city "soon". I need your help is testing the police call boxes.<br><br>
        Speed is most important here. We don't know when they will arrive. So get your Super Speed, Super Jump or teleport binds ready!<br><br>
        WHAT?! You don't have any of those powers?! Well.....do what you can.]],heroName),
        buttons = {
            button01 = {"Help test","CONTACTLINK_HELLO"},
            button02 = {"Leave","CONTACTLINK_BYE"}
        }
    }
end


RelayRace.startDialogs = function()
    Player.SetActiveDialogCallback(contactsForZone.RelayRace.callback);
    local contactStatus = Contacts.FindContactByName("Sgt. Johnson");
    
    if(contactStatus ~= false) then
        printDebug("RelayRace.StartDialogs: dialogScreenIdx: " .. tostring(contactStatus.dialogScreenIdx));
        if(contactStatus.dialogScreenIdx >=  1 or contactStatus.currentStanding >= 1) then
            MapClientSession.Contact_dialog(contactsForZone.RelayRace.dialogPages[4].contactDialog.message, contactsForZone.RelayRace.dialogPages[4].contactDialog.buttons);
        else
            MapClientSession.Contact_dialog(contactsForZone.RelayRace.dialogPages[1].contactDialog.message, contactsForZone.RelayRace.dialogPages[1].contactDialog.buttons);
        end
    elseif(contactStatus == false) then
        contactStatus = contactsForZone.RelayRace.CreateContact();
        Player.AddUpdateContact(contactStatus);
        MapClientSession.Contact_dialog(contactsForZone.RelayRace.dialogPages[1].contactDialog.message, contactsForZone.RelayRace.dialogPages[1].contactDialog.buttons);
    end
end

RelayRace.callback = function(id)
    local button = Contacts.ParseContactButton(id);
    printDebug("RelayRace.Callback Button String: " .. tostring(button));
    
    local contactStatus = Contacts.FindContactByName("Sgt. Johnson");
    printDebug("RelayRace.callback: dialogScreenIdx: " .. tostring(contactStatus.dialogScreenIdx));
    if(contactStatus ~= false) then
        if(button == "CONTACTLINK_INTRODUCE_CONTACT1") then
            contactStatus = contactsForZone.RelayRace.CreateContact();
            Player.AddUpdateContact(contactStatus);
            MapClientSession.Contact_dialog(contactsForZone.RelayRace.dialogPages[1].contactDialog.message, contactsForZone.RelayRace.dialogPages[1].contactDialog.buttons);
        end
        if(contactStatus.dialogScreenIdx == 0) then
            if(button == "CONTACTLINK_HELLO") then
                MapClientSession.Contact_dialog(contactsForZone.RelayRace.dialogPages[2].contactDialog.message, contactsForZone.RelayRace.dialogPages[2].contactDialog.buttons);
            elseif(button == "CONTACTLINK_ACCEPTSHORT") then
                --contactsForZone.RelayRace.tasks[1].dbId = contactsForZone.RelayRace.entityId;
                --Player.AddUpdateTask(contactsForZone.RelayRace.tasks[1]);

                MapInstance.SetOnTickCallback(Player.entityId, contactsForZone.RelayRace.onTickCallBack);
                MapInstance.StartTimer(Player.entityId);

                contactStatus.dialogScreenIdx = 1;
                Player.AddUpdateContact(contactStatus);
                Player.CloseContactDialog();
            elseif(button == "CONTACTLINK_WRONGMODE") then
                MapClientSession.Contact_dialog(contactsForZone.RelayRace.dialogPages[3].contactDialog.message, contactsForZone.RelayRace.dialogPages[3].contactDialog.buttons);
            end

        elseif(contactStatus.dialogScreenIdx == 1) then
            if(button == "CONTACTLINK_ACCEPTSHORT") then
                --local task = DeepCopy(contactsForZone.RelayRace.tasks[1]);
                --task.description = "Test police call box 2";
                --task.location.location = DeepCopy(contactsForZone.RelayRace.locations[2].coordinates);
               -- Player.AddUpdateTask(task);

                MapInstance.SetOnTickCallback(Player.entityId, contactsForZone.RelayRace.onTickCallBack);
                MapInstance.StartTimer(Player.entityId);

            end
            
        end
    end
end

RelayRace.onTickCallBack = function(start, diff, current)
    --printDebug("startTime: " .. tostring(startTime) .. " diff: " .. tostring(diff) .. " current: " .. tostring(current));
    --contactsForZone.RelayRace.time = diff;

    local contactStatus = Contacts.FindContactByName("Sgt. Johnson");
    
    if(contactStatus ~= false) then
        --printDebug("RelayRace.onTickCallBack: dialogScreenIdx: " .. tostring(contactStatus.dialogScreenIdx));
        if(contactStatus.dialogScreenIdx ==  1 and contactStatus.currentStanding == 1) then
            MapInstance.StopTimer(Player.entityId);
            Player.SetActiveDialogCallback(contactsForZone.RelayRace.callback);

            local statistic = characterStatistic.new();
            statistic.name = "Relay Race";
            statistic.id = 11; -- id for race type
            statistic.time = DeepCopy(diff);
            Player.AddStatistic(statistic);

            MapInstance.ClearTimer(Player.entityId);
            local formatedTime = SecondsToClock(diff);
            RelayRace.dialogPages[5] = {};
            RelayRace.dialogPages[5].contactDialog = {
                message = string.format([[Good job. Your time was %s.<br><br>Ok. On to the 2nd call box. This one is to the north east,
                just south of a park in Hyperion Way. Now move fast! We don't have much time!<br><br>
                <color #2189b9>Your time starts once you click "ready".</color>]], formatedTime),
                buttons = {
                    button01 = {"Ready","CONTACTLINK_ACCEPTSHORT"}
                }
            }
            MapClientSession.Contact_dialog(contactsForZone.RelayRace.dialogPages[5].contactDialog.message, contactsForZone.RelayRace.dialogPages[5].contactDialog.buttons);

            contactStatus.currentStanding = 2;
            Player.AddUpdateContact(contactStatus);

        end
    end
    
end


RelayRace.CreateContact = function()
    
    local contact = Contact.new();
    contact.name = "Sgt. Johnson";
    contact.currentStanding = 0;
    contact.notifyPlayer = false;
    contact.npcId = 2009;
    contact.contactIdx = contactsForZone.RelayRace.entityId;
    contact.hasLocation = true;
    contact.taskIndex = 0;
    contact.locationDescription = "Atlas Park";
    contact.location = Destination.new();
    contact.location.location = vec3.new(192, 0, -117);
    contact.location.name = "johnson_location";
    contact.location.mapName =  "Atlas Park";
    contact.confidantThreshold = 2;
    contact.friendThreshold = 4;
    contact.completeThreshold = 6;
    contact.canUseCell = false;
    contact.dialogScreenIdx = 0;

    return contact;
end

-- Must be at end
if(contactsForZone ~= nil) then
    contactsForZone.RelayRace = RelayRace;
end

printDebug('relayRace script loaded');

--[[     
                Notes
    
Polce boxes

"Patrol_Easy_1_City_01_01" "(461,3.05176e-5,778)"
"Patrol_Easy_2_City_01_01" "(-928,-15.8626,44.5)"
"Patrol_Easy_3_City_01_01" "(89,0,-1785)"
"Patrol_Easy_4_City_01_01" "(679,42,-1246.5)"
"Patrol_Easy_5_City_01_01" "(1099.5,0,124.5)"


"Patrol_Hard_1_City_01_01" "(-1672.5,-0.000366211,-541.5)"
"Patrol_Hard_2_City_01_01" "(-1670,-7.62939e-6,-1605.5)"
"Patrol_Hard_3_City_01_01" "(-25.5,-1.52588e-5,-2190.5)"
"Patrol_Hard_4_City_01_01" "(2052,-0.000366211,-1923.5)"
"Patrol_Hard_5_City_01_01" "(2284.5,-32,540.5)"

]]