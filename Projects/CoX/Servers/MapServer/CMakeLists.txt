if(ENABLE_SCRIPTING_ENGINE)
    set(SCRIPTING_CPP ScriptingEngine.cpp)
else()
    set(SCRIPTING_CPP ScriptingEngine_Null.cpp)
endif()
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
    SlashCommand.cpp
    WorldSimulation.cpp
    ${SCRIPTING_CPP}
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
add_handler(MapServer ${target_SOURCES})
target_link_libraries(MapServer PUBLIC CRUDP_Protocol common_runtime)
target_link_libraries(MapServer PUBLIC Qt5::Core)
target_link_libraries(MapServer PUBLIC gameData)

if(ENABLE_SCRIPTING_ENGINE)
    target_compile_definitions(MapServer PUBLIC -DSCRIPTING_ENABLED=1)
    target_link_libraries(MapServer INTERFACE lua_IMP sol2_IMP)
endif()
if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()
# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES MapServer SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${target_CPP} ${target_INCLUDE}) 
set_target_properties (MapServer PROPERTIES
    FOLDER Projects/CoX/Servers
)
