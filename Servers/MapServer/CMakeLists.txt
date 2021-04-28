
SET(target_CPP
    CritterGenerator.cpp
    DataHelpers.cpp
    EntityStorage.cpp
    EntityUpdateCodec.cpp
    MapClientSession.cpp
    NpcGenerator.cpp
    MapEventFactory.cpp
    MapInstance.cpp
    MapLink.cpp
    MapManager.cpp
    MapSceneGraph.cpp
    MapServer.cpp
    MapTemplate.cpp
    MessageHelpers.cpp
    NetCommandManager.cpp
    NetFxHelpers.cpp
    WorldSimulation.cpp
)
SET(target_INCLUDE
    CritterGenerator.h
    DataHelpers.h
    EntityStorage.h
    EntityUpdateCodec.h
    NpcGenerator.h
    MapEventFactory.h
    MapInstance.h
    MapLink.h
    MapManager.h
    MapSceneGraph.h
    MapServer.h
    MapTemplate.h
    MessageHelpers.h
    NetCommandManager.h
    NetFxHelpers.h
    WorldSimulation.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR} .)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_handler(MapServer ${target_SOURCES})
target_link_libraries(MapServer PUBLIC CRUDP_Protocol common_runtime)
target_link_libraries(MapServer PUBLIC Qt5::Core)
target_link_libraries(MapServer PUBLIC gameData)

add_subdirectory(ScriptingEngine)
add_subdirectory(SlashCommands)

target_link_libraries(MapServer PUBLIC Qt5::Core)
target_link_libraries(MapServer PRIVATE SlashCommands)


if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()
# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES MapServer SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${target_CPP} ${target_INCLUDE}) 
set_target_properties (MapServer PROPERTIES
    FOLDER Servers
)
