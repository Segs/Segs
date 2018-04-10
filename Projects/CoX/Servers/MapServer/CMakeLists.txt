if(ENABLE_SCRIPTING_ENGINE)
    set(SCRIPTING_CPP ScriptingEngine.cpp)
else()
    set(SCRIPTING_CPP ScriptingEngine_Null.cpp)
endif()
SET(target_CPP
    DataHelpers.cpp
    EntityStorage.cpp
    EntityUpdateCodec.cpp
    Events/GameCommandList.cpp
    Events/ChatDividerMoved.cpp
    Events/ChatMessage.cpp
    Events/EntitiesResponse.cpp
    Events/InputState.cpp
    Events/KeybindSettings.cpp
    Events/LocationVisited.cpp
    Events/PlaqueVisited.cpp
    Events/NewEntity.cpp
    Events/SaveClientOptions.cpp
    Events/SceneEvent.cpp
    Events/InfoMessageCmd.cpp
    Events/WindowState.cpp
    MapClient.cpp
    MapEventFactory.cpp
    MapInstance.cpp
    MapLink.cpp
    MapManager.cpp
    MapServer.cpp
    MapServerData.cpp
    MapTemplate.cpp
    NetCommandManager.cpp
    SlashCommand.cpp
    WorldSimulation.cpp
    ${SCRIPTING_CPP}
)
SET(target_INCLUDE
    DataHelpers.h
    EntityStorage.h
    EntityUpdateCodec.h
    Events/GameCommandList.h
    Events/ChatDividerMoved.h
    Events/ChatMessage.h
    Events/EmailHeaders.h
    Events/EmailRead.h
    Events/EntitiesResponse.h
    Events/FloatingDamage.h
    Events/FriendsListUpdate.h
    Events/InputState.h
    Events/KeybindSettings.h
    Events/LocationVisited.h
    Events/MapEventTypes.h
    Events/MessageChannels.h
    Events/NewEntity.h
    Events/PlaqueVisited.h
    Events/PlayerInfo.h
    Events/SaveClientOptions.h
    Events/SceneEvent.h
    Events/Shortcuts.h
    Events/SidekickOffer.h
    Events/StandardDialogCmd.h
    Events/TeamLooking.h
    Events/TeamOffer.h
    Events/InfoMessageCmd.h
    Events/WindowState.h
    MapClient.h
    MapEventFactory.h
    MapEvents.h
    MapInstance.h
    MapLink.h
    MapManager.h
    MapRef.h
    MapServer.h
    MapServerData.h
    MapTemplate.h
    NetCommandManager.h
    ScriptingEngine.h
    SlashCommand.h
    WorldSimulation.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR} .)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
ADD_LIBRARY(mapserver_lib STATIC ${target_SOURCES})
target_link_libraries(mapserver_lib PUBLIC adminserver_lib CRUDP_Protocol)
qt5_use_modules(mapserver_lib LINK_PUBLIC Core)
if(ENABLE_SCRIPTING_ENGINE)
    target_compile_definitions(mapserver_lib PUBLIC -DSCRIPTING_ENABLED=1)
    target_link_libraries(mapserver_lib INTERFACE lua_IMP sol2_IMP)
endif()

