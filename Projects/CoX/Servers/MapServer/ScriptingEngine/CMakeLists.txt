if(ENABLE_SCRIPTING_ENGINE)
    set(SCRIPTING_CPP ScriptingEngine.cpp)
    set(SCRIPTING_REGISTERTYPES_CPP ScriptingEngine_RegisterTypes.cpp)
    set(SCRIPTING_HELPERS_CPP ScriptingEngine_Helpers.cpp)
    set(SCRIPTING_PRIVATE_CPP ScriptingEnginePrivate.cpp)
    set(SCRIPTING_PRIVATE_H ScriptingEnginePrivate.h)
else()
    set(SCRIPTING_CPP ScriptingEngine_Null.cpp)
endif()

SET(target_CPP
    ${SCRIPTING_CPP}
    ${SCRIPTING_REGISTERTYPES_CPP}
    ${SCRIPTING_HELPERS_CPP}
    ${SCRIPTING_PRIVATE_CPP}
)
SET(target_INCLUDE
    ScriptingEngine.h
    ${SCRIPTING_PRIVATE_H}
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

add_library(ScriptingEngine ${target_SOURCES})
target_link_libraries(ScriptingEngine PUBLIC Qt5::Core gameData)

if(ENABLE_SCRIPTING_ENGINE)
    target_compile_definitions(ScriptingEngine PUBLIC -DSCRIPTING_ENABLED=1)
    target_link_libraries(ScriptingEngine INTERFACE lua_IMP sol2_IMP)
endif()

if(ENABLE_TESTS)
#    add_subdirectory(UnitTests)
endif()
get_target_property(TARGET_SOURCES ScriptingEngine SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES})
set_target_properties (ScriptingEngine PROPERTIES
    FOLDER Projects/CoX/Servers
)
