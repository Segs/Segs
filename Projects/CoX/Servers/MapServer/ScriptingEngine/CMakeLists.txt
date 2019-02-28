if(ENABLE_SCRIPTING_ENGINE)
    set(SCRIPTING_CPP ScriptingEngine.cpp)
else()
    set(SCRIPTING_CPP ScriptingEngine_Null.cpp)
endif()
SET(target_CPP
    ${SCRIPTING_CPP}
    ScriptingEnginePrivate.cpp
    ScriptingEngine_RegisterTypes.cpp
    ScriptingEngine_Helpers.cpp
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

add_handler(ScriptingEngine ${target_SOURCES})
target_link_libraries(ScriptingEngine PUBLIC Qt5::Core gameData)
target_link_libraries(ScriptingEngine INTERFACE SEGS_Components )

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
