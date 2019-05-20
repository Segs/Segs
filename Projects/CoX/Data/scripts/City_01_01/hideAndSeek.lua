-- ATLAS PARK
printDebug("hideAndSeek script loading...");

local HideAndSeek = {};
HideAndSeek.spawned = false;
HideAndSeek.name = "Ashley Adams";
HideAndSeek.model = "model_pennyPreston";
HideAndSeek.location = {};
HideAndSeek.location.coordinates = vec3.new(80, 16, -247);
HideAndSeek.location.orientation = vec3.new(0, 0, 0);

HideAndSeek.variation = 1;
HideAndSeek.expected = false;
HideAndSeek.entityId = nil;
HideAndSeek.minLevel = 0;
HideAndSeek.hidingPlaceId = 0;
HideAndSeek.hideTime = 0;
HideAndSeek.checkInTime = 0;


HideAndSeek.isStore = false;

HideAndSeek.dialogPages = {};
HideAndSeek.dialogPages[1] = {};
HideAndSeek.dialogPages[1].contactDialog = {
    message = [[<img src="npc:1948" align="left"><br>OH MY! A HERO! I haven't seen one of you for years.<br>
    Lets play a game, for old times sake? I'll hide somewhere in Atlas Park and you try to find me.<br><br>
    Everyone can play!<br>]],
    buttons = {
        button01 = {"Play game","CONTACTLINK_MISSIONS"},
        button02 = {"", ""},
        button03 = {"Leave", "CONTACTLINK_BYE"}
    }
}

HideAndSeek.dialogPages[2] = {};
HideAndSeek.dialogPages[2].contactDialog = {
    message = [[<img src="npc:1948" align="left"><br>YAY!<br><br>
    Ok. I'll give everyone a "clue" to help find me. Ready?]],
    buttons = {
        button01 = {"Ready","CONTACTLINK_ACCEPTSHORT"},
        button02 = {"", ""},
        button03 = {"Not playing", "CONTACTLINK_MAIN"}
    }
}

HideAndSeek.dialogPages[3] = {};
HideAndSeek.dialogPages[3].contactDialog = {
    message = [[<img src="npc:1948" align="left"><br>Oh.....You're no fun!]],
    buttons = {
        button01 = {"Leave","CONTACTLINK_BYE"}
    }
}

--Extra global variables
HideAndSeek.isHidden = false;
HideAndSeek.hidingPlaces = {};
HideAndSeek.hidingPlaces[10] = {};
HideAndSeek.hidingPlaces[10].coordinates = vec3.new(84, 27, -400);
HideAndSeek.hidingPlaces[10].orientation = vec3.new(0, 3.14, 0);
HideAndSeek.hidingPlaces[10].clue = "That globe looks heavy. I hope this statue doesn't step on me. hehehe";
HideAndSeek.hidingPlaces[11] = {};
HideAndSeek.hidingPlaces[11].coordinates = vec3.new(336,-15.8, 983);
HideAndSeek.hidingPlaces[11].orientation = vec3.new(0, 3.14, 0);
HideAndSeek.hidingPlaces[11].clue = "Boy this place smells. Good thing the Vahzilok aren't around.";
HideAndSeek.hidingPlaces[12] = {};
HideAndSeek.hidingPlaces[12].coordinates = vec3.new(211, 0, -1527);
HideAndSeek.hidingPlaces[12].orientation = vec3.new(0, 0, 0);
HideAndSeek.hidingPlaces[12].clue = "I love the superlanes! You can't hear the trains over the pins falling.";
HideAndSeek.hidingPlaces[13] = {};
HideAndSeek.hidingPlaces[13].coordinates = vec3.new(-17, -768, -639);
HideAndSeek.hidingPlaces[13].orientation = vec3.new(0, 1.58, 0);
HideAndSeek.hidingPlaces[13].clue = "Now that the heroes are back, we need people in New Hero Assignment!";
HideAndSeek.hidingPlaces[14] = {};
HideAndSeek.hidingPlaces[14].coordinates = vec3.new(-102, -174, -1655);
HideAndSeek.hidingPlaces[14].orientation = vec3.new(0, 1.58, 0);
HideAndSeek.hidingPlaces[14].clue = "Why isn't there a doctor here?";
HideAndSeek.hidingPlaces[15] = {};
HideAndSeek.hidingPlaces[15].coordinates = vec3.new(2857, 0.6, -1855);
HideAndSeek.hidingPlaces[15].orientation = vec3.new(0, 4.74, 0);
HideAndSeek.hidingPlaces[15].clue = "Were are the police? Perez Park is dangerous.";
HideAndSeek.hidingPlaces[16] = {};
HideAndSeek.hidingPlaces[16].coordinates = vec3.new(640, 74, -1416);
HideAndSeek.hidingPlaces[16].orientation = vec3.new(0, 4.74, 0);
HideAndSeek.hidingPlaces[16].clue = "I hear the trains but they aren't at the station. Where are they?";
HideAndSeek.hidingPlaces[17] = {};
HideAndSeek.hidingPlaces[17].coordinates = vec3.new(972, -27, -765);
HideAndSeek.hidingPlaces[17].orientation = vec3.new(0, 0, 0);
HideAndSeek.hidingPlaces[17].clue = "I never understood how a lake counts as a park.";
HideAndSeek.hidingPlaces[18] = {};
HideAndSeek.hidingPlaces[18].coordinates = vec3.new(-1791, 12, -2430);
HideAndSeek.hidingPlaces[18].orientation = vec3.new(0, 0.79, 0);
HideAndSeek.hidingPlaces[18].clue = "I don't think I've ever gone this far in The Promenade. Was this S,W,N,E thing?";
HideAndSeek.hidingPlaces[19] = {};
HideAndSeek.hidingPlaces[19].coordinates = vec3.new(-1176, 0, -511);
HideAndSeek.hidingPlaces[19].orientation = vec3.new(0, 0.79, 0);
HideAndSeek.hidingPlaces[19].clue = "Hyperion Way has some of my favorite music. hehehe";
HideAndSeek.hidingPlaces[19] = {};
HideAndSeek.hidingPlaces[19].coordinates = vec3.new(-1429, 0, 176);
HideAndSeek.hidingPlaces[19].orientation = vec3.new(0, 4.71, 0);
HideAndSeek.hidingPlaces[19].clue = [[What's "Scheherazde"? Hyperion way has some strange places.]];


HideAndSeek.startingLocationBackup = {};
HideAndSeek.startingLocationBackup = DeepCopy(HideAndSeek.location); -- backup for when she moves.

--end Extras

HideAndSeek.createContactDialogsWithHeroName = function (heroName)
    --when found sometimes?
    contactsForZone.HideAndSeek.dialogPages[5] = {};
    contactsForZone.HideAndSeek.dialogPages[5].contactDialog = {
        message = string.format([[<img src="npc:1948" align="left"><br>Oh %s! You found me!<br><br>
        I'll head back to the stairs at Atlas Park so we can play again.]],heroName),
        buttons = {
            button01 = {"Leave","CONTACTLINK_IDENTIFYCLUE"}
        }
    }
end


HideAndSeek.startDialogs = function()
    Player.SetActiveDialogCallback(contactsForZone.HideAndSeek.callback);
   

    if(contactsForZone.HideAndSeek.isHidden == true) then
        --Random response?
        MapClientSession.Contact_dialog(contactsForZone.HideAndSeek.dialogPages[5].contactDialog.message, contactsForZone.HideAndSeek.dialogPages[5].contactDialog.buttons);
    else
        MapClientSession.Contact_dialog(contactsForZone.HideAndSeek.dialogPages[1].contactDialog.message, contactsForZone.HideAndSeek.dialogPages[1].contactDialog.buttons);
    end
end

HideAndSeek.callback = function(id)
    local button = Contacts.ParseContactButton(id);
    printDebug("HideAndSeek.Callback Button String: " .. tostring(button));
    if(button == "CONTACTLINK_MISSIONS") then
        MapClientSession.Contact_dialog(contactsForZone.HideAndSeek.dialogPages[2].contactDialog.message, contactsForZone.HideAndSeek.dialogPages[2].contactDialog.buttons);
    elseif (button == "CONTACTLINK_ACCEPTSHORT") then
        -- pick hiding place
        math.randomseed(DateTime.SecsSince2000Epoch());
        contactsForZone.HideAndSeek.hidingPlaceId = math.random(10, 19);
      
        --broadcast clue
        MapInstance.NpcMessage('b', contactsForZone.HideAndSeek.entityId, contactsForZone.HideAndSeek.hidingPlaces[contactsForZone.HideAndSeek.hidingPlaceId].clue);

        -- move -- Remove/respawn
        Player.CloseContactDialog();
        MapInstance.RemoveNpc(contactsForZone.HideAndSeek.entityId);
        contactsForZone.HideAndSeek.spawned = false;
        contactsForZone.HideAndSeek.location = DeepCopy(contactsForZone.HideAndSeek.hidingPlaces[contactsForZone.HideAndSeek.hidingPlaceId]);
        Contacts.SpawnContacts('Atlas Park');
        contactsForZone.HideAndSeek.isHidden = true;
        contactsForZone.HideAndSeek.hideTime = DateTime.SecsSince2000Epoch();
        contactsForZone.HideAndSeek.checkInTime = DateTime.SecsSince2000Epoch();
        MapInstance.SetOnTickCallback(contactsForZone.HideAndSeek.entityId, contactsForZone.HideAndSeek.onTickCallBack);
        MapInstance.StartTimer(contactsForZone.HideAndSeek.entityId);
        

    elseif(button == "CONTACTLINK_MAIN") then
        MapClientSession.Contact_dialog(contactsForZone.HideAndSeek.dialogPages[3].contactDialog.message, contactsForZone.HideAndSeek.dialogPages[3].contactDialog.buttons);
    elseif(button == "CONTACTLINK_IDENTIFYCLUE") then
        MapInstance.StopTimer(contactsForZone.HideAndSeek.entityId);
        --give point

        Player.AddHideAndSeekPoint(1);-- Server will increment count.
        Player.CloseContactDialog();
        MapInstance.NpcMessage('b', contactsForZone.HideAndSeek.entityId, string.format("Heroes! %s found me. I'll be back at the steps in Atlas Plaza if you want to play again.", heroName));
        MapInstance.RemoveNpc(contactsForZone.HideAndSeek.entityId);
        contactsForZone.HideAndSeek.spawned = false;
        contactsForZone.HideAndSeek.location = DeepCopy(contactsForZone.HideAndSeek.startingLocationBackup);
        Contacts.SpawnContacts('Atlas Park');
        contactsForZone.HideAndSeek.isHidden = false;
        

    end
end

HideAndSeek.onTickCallBack = function(startTime, diff, current)
    --printDebug("startTime: " .. tostring(startTime) .. " diff: " .. tostring(diff) .. " current: " .. tostring(current));
    local stopTime15 = 900; -- 15 minutes 900 seconds
    local stopTime30 = 1800; -- 30 minutes  1800 seconds
    local resetCheckInTimer = contactsForZone.HideAndSeek.checkInTime + 60;
    
    -- Once we have a "proximity check" this wouldn't need to run every minute.
    -- It could be sent when a player is within X range.
    if(contactsForZone.HideAndSeek.isHidden and current == resetCheckInTimer) then
        MapInstance.NpcMessage('l', contactsForZone.HideAndSeek.entityId, "Is someone there?");
        contactsForZone.HideAndSeek.checkInTime = DeepCopy(resetCheckInTimer);
    end

    if (contactsForZone.HideAndSeek.isHidden and diff == stopTime15) then
        MapInstance.NpcMessage('b', contactsForZone.HideAndSeek.entityId, "Heeeerrroooooesssss. Come on! Its been 15 minutes!");
        MapInstance.NpcMessage('b', contactsForZone.HideAndSeek.entityId, contactsForZone.HideAndSeek.hidingPlaces[contactsForZone.HideAndSeek.hidingPlaceId].clue);

    elseif (contactsForZone.HideAndSeek.isHidden and diff == stopTime30) then
        MapInstance.StopTimer(contactsForZone.HideAndSeek.entityId);
        MapInstance.NpcMessage('b', contactsForZone.HideAndSeek.entityId, "I guess I win this one. hehehe");
        MapInstance.NpcMessage('b', contactsForZone.HideAndSeek.entityId, "I'll be back at the steps in Atlas Plaza if you want to play again.");

        MapInstance.ClearTimer(contactsForZone.HideAndSeek.entityId);
        contactsForZone.HideAndSeek.spawned = false;
        contactsForZone.HideAndSeek.location = DeepCopy(contactsForZone.HideAndSeek.startingLocationBackup);
        MapInstance.RemoveNpc(contactsForZone.HideAndSeek.entityId);
        Contacts.SpawnContacts('Atlas Park');
        contactsForZone.HideAndSeek.isHidden = false;
        
        
    end
end

-- Must be at end
if(contactsForZone ~= nil) then
    contactsForZone.HideAndSeek = HideAndSeek;
end
printDebug("hideAndSeek script loaded");