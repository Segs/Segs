printDebug('Loading contacts.lua...')

contactsForZone = {};

-- Begin Contact Scripts
include_lua('luaBot.lua');
include_lua('hideAndSeek.lua');
include_lua('relayRace.lua');
include_lua('TimeCop.lua')

-- End Contact scripts


printDebug('Finished Loading contacts.lua')