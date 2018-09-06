if(ENABLE_SCRIPTING_ENGINE)
    set(SCRIPTING_CPP ScriptingEngine.cpp)
else()
    set(SCRIPTING_CPP ScriptingEngine_Null.cpp)
endif()
SET(target_CPP
    DataHelpers.cpp
    EntityStorage.cpp
    EntityUpdateCodec.cpp
    Events/ChatDividerMoved.cpp
    Events/ChatMessage.cpp
    Events/EntitiesResponse.cpp
    Events/InputState.cpp
    Events/LocationVisited.cpp
    Events/PlaqueVisited.cpp
    Events/NewEntity.cpp
    Events/SaveClientOptions.cpp
    Events/SceneEvent.cpp
    Events/InfoMessageCmd.cpp
    Events/WindowState.cpp
    NpcGenerator.cpp
    MapEventFactory.cpp
    MapInstance.cpp
    MapLink.cpp
    MapManager.cpp
    MapSceneGraph.cpp
    MapServer.cpp
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
    Events/ChangeTitle.h
    Events/ChatDividerMoved.h
    Events/ChatMessage.h
    Events/ClientStates.h
    Events/DeadNoGurney.h
    Events/EmailHeaders.h
    Events/EmailMessageStatus.h
    Events/EmailRead.h
    Events/EntitiesResponse.h
    Events/FloatingDamage.h
    Events/FloatingInfo.h
    Events/FloatingInfoStyles.h
    Events/FriendsListUpdate.h
    Events/InputState.h
    Events/LocationVisited.h
    Events/LevelUp.h
    Events/MapEventTypes.h
    Events/MapXferList.h
    Events/MessageChannels.h
    Events/NewEntity.h
    Events/MapSessionReference.h
    Events/PlaqueVisited.h
    Events/PlayerInfo.h
    Events/PowerSystemEvents.h
    Events/SaveClientOptions.h
    Events/SceneEvent.h
    Events/SetClientState.h
    Events/Shortcuts.h
    Events/SidekickOffer.h
    Events/StandardDialogCmd.h
    Events/TeamLooking.h
    Events/TeamOffer.h
    Events/TrayAdd.h
    Events/InfoMessageCmd.h
    Events/InteractWithEntity.h
    Events/WindowState.h
    NpcGenerator.h
    MapEventFactory.h
    MapEvents.h
    MapInstance.h
    MapLink.h
    MapManager.h
    MapRef.h
    MapSceneGraph.h
    MapServer.h
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
scan_event_types(MapEvents ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Events)
add_event_sources(MapEvents target_SOURCES)
ADD_LIBRARY(mapserver_lib STATIC ${target_SOURCES})
target_link_libraries(mapserver_lib PUBLIC CRUDP_Protocol common_runtime)
target_link_libraries(mapserver_lib PUBLIC Qt5::Core)
target_link_libraries(mapserver_lib PUBLIC network_structures)
#target_link_libraries(mapserver_lib PUBLIC gameData game_db_sync_lib ace_IMP network_structures)
if(ENABLE_SCRIPTING_ENGINE)
    target_compile_definitions(mapserver_lib PUBLIC -DSCRIPTING_ENABLED=1)
    target_link_libraries(mapserver_lib INTERFACE lua_IMP sol2_IMP)
endif()
if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()

