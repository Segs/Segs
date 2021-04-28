
SET(target_CPP
    ScriptingEngine.cpp
    ScriptingEngine_CharacterTypes.cpp
    ScriptingEngine_GenericTypes.cpp
    ScriptingEngine_Helpers.cpp
    ScriptingEngine_SpawnerTypes.cpp
    ScriptingEnginePrivate.cpp
)
SET(target_INCLUDE
    ScriptingEngine.h
    ScriptingEnginePrivate.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
    ${target_CPP}
    ${target_INCLUDE}
)

add_library(ScriptingEngine ${target_SOURCES})
target_link_libraries(ScriptingEngine PUBLIC Qt5::Core gameData)

target_compile_definitions(ScriptingEngine PUBLIC -DSCRIPTING_ENABLED=1)
target_link_libraries(ScriptingEngine PUBLIC lua_IMP sol2)

if(ENABLE_TESTS)
#    add_subdirectory(UnitTests)
endif()
get_target_property(TARGET_SOURCES ScriptingEngine SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES})
set_target_properties (ScriptingEngine PROPERTIES
    FOLDER Servers
)
