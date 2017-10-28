if(ENABLE_SCRIPTING_ENGINE)
    set(SCRIPTING_CPP ${CMAKE_CURRENT_SOURCE_DIR}/ScriptingEngine.cpp)
else()
    set(SCRIPTING_CPP ${CMAKE_CURRENT_SOURCE_DIR}/ScriptingEngine_Null.cpp)
endif()
SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/DataHelpers.cpp
${CMAKE_CURRENT_SOURCE_DIR}/EntityStorage.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Events/ChatDividerMoved.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Events/ChatMessage.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Events/EntitiesResponse.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Events/InputState.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Events/NewEntity.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Events/SceneEvent.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapClient.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapEventFactory.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapInstance.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapLink.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapServerData.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapTemplate.cpp
${CMAKE_CURRENT_SOURCE_DIR}/NetCommandManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/WorldSimulation.cpp
${SCRIPTING_CPP}
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/DataHelpers.h
${CMAKE_CURRENT_SOURCE_DIR}/EntityStorage.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/ChatDividerMoved.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/ChatMessage.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/EntitiesResponse.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/InputState.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/NewEntity.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/SceneEvent.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/Shortcuts.h
${CMAKE_CURRENT_SOURCE_DIR}/MapClient.h
${CMAKE_CURRENT_SOURCE_DIR}/MapEventFactory.h
${CMAKE_CURRENT_SOURCE_DIR}/MapEvents.h
${CMAKE_CURRENT_SOURCE_DIR}/MapInstance.h
${CMAKE_CURRENT_SOURCE_DIR}/MapLink.h
${CMAKE_CURRENT_SOURCE_DIR}/MapManager.h
${CMAKE_CURRENT_SOURCE_DIR}/MapRef.h
${CMAKE_CURRENT_SOURCE_DIR}/MapServer.h
${CMAKE_CURRENT_SOURCE_DIR}/MapTemplate.h
${CMAKE_CURRENT_SOURCE_DIR}/NetCommandManager.h
${CMAKE_CURRENT_SOURCE_DIR}/ScriptingEngine.h
${CMAKE_CURRENT_SOURCE_DIR}/WorldSimulation.h
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

