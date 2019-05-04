--TimeCop controls auto-refreshing of map spawners (spawndef-based)
local TimeCop = {};
TimeCop.spawned = false;
TimeCop.name = "Time Cop";
TimeCop.model = "MaleNpc_230";
TimeCop.location = {};
TimeCop.location.coordinates = vec3.new(6212.5, 0, 2700);
TimeCop.location.orientation = vec3.new(0, 3.14, 0);

TimeCop.variation = 1;
TimeCop.entityId = nil;
TimeCop.minLevel = 0;
TimeCop.hideTime = 0;
TimeCop.isStore = false;
TimeCop.NumEncounters = 50;     --number of encounters to respawn
TimeCop.RefreshRate = 60;       --number of seconds before respawns

TimeCop.onTickCallBack = function(startTime, diff, current)
    --print("startTime: " .. tostring(startTime) .. " diff: " .. tostring(diff) .. " current: " .. tostring(current));
    local stopTime = TimeCop.RefreshRate;

    if(diff == stopTime) then
        MapInstance.StopTimer(contactsForZone.TimeCop.entityId); 
        DespawnMapEncounters()
        RandomSpawn(TimeCop.NumEncounters)
        MapInstance.StartTimer(contactsForZone.TimeCop.entityId); 
    end
end

--Default with no arg turns Time Cop off. Otherwise, pass "true" to activate
function TimeCopMode(Refresh, EncounterCount, Rate)
    EncounterCount = EncounterCount or TimeCop.NumEncounters
    Rate = Rate or TimeCop.RefreshRate

    TimeCop.NumEncounters = EncounterCount
    TimeCop.RefreshRate = Rate

    if Refresh == nil or Refresh == false then
        MapInstance.StopTimer(contactsForZone.TimeCop.entityId);
        MapClientSession.SendInfoMessage(13, "Map encounters auto-refresh has ceased.")
    elseif Refresh == true then

        MapInstance.StartTimer(contactsForZone.TimeCop.entityId); 
        MapClientSession.SendInfoMessage(13, "Map encounters will auto-refresh.")
    end
end

-- Must be at end
if(contactsForZone ~= nil) then
    contactsForZone.TimeCop = TimeCop;
end

