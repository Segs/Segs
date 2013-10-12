SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/MapManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapTemplate.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapInstance.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapClient.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/EntityStorage.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapEventFactory.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapLink.cpp
${CMAKE_CURRENT_SOURCE_DIR}/NetCommandManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Events/NewEntity.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Events/SceneEvent.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Events/EntitiesResponse.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Events/ChatMessage.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Events/InputState.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/NetCommandManager.h
${CMAKE_CURRENT_SOURCE_DIR}/MapManager.h
${CMAKE_CURRENT_SOURCE_DIR}/MapTemplate.h
${CMAKE_CURRENT_SOURCE_DIR}/MapInstance.h
${CMAKE_CURRENT_SOURCE_DIR}/EntityStorage.h
${CMAKE_CURRENT_SOURCE_DIR}/MapClient.h
${CMAKE_CURRENT_SOURCE_DIR}/MapHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/MapLink.h
${CMAKE_CURRENT_SOURCE_DIR}/MapEvents.h
${CMAKE_CURRENT_SOURCE_DIR}/MapRef.h
${CMAKE_CURRENT_SOURCE_DIR}/MapServer.h
${CMAKE_CURRENT_SOURCE_DIR}/MapEventFactory.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/NewEntity.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/SceneEvent.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/ChatMessage.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/EntitiesResponse.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/InputState.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/Shortcuts.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(target_DEPENDS "")

SEGS_ADD_COMPONENT(cox_server_common LIB)
SEGS_ADD_COMPONENT(adminserver_lib LIB) # used to access character db

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR} .)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
ADD_LIBRARY(mapserver_lib STATIC ${target_SOURCES})
ADD_DEPENDENCIES(mapserver_lib ${target_DEPENDS})

SEGS_REGISTER_COMPONENT(mapserver_lib "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )

