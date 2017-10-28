SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/HashStorage.cpp
${CMAKE_CURRENT_SOURCE_DIR}/CharacterClient.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameEvents.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameEventFactory.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameLink.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/HashStorage.h
${CMAKE_CURRENT_SOURCE_DIR}/CharacterClient.h
${CMAKE_CURRENT_SOURCE_DIR}/GameServer.h
${CMAKE_CURRENT_SOURCE_DIR}/GameServerData.h
${CMAKE_CURRENT_SOURCE_DIR}/GameHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/GameEventFactory.h
${CMAKE_CURRENT_SOURCE_DIR}/GameEvents.h
${CMAKE_CURRENT_SOURCE_DIR}/GameLink.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(target_DEPENDS "")

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
ADD_LIBRARY(gameserver_lib STATIC ${target_SOURCES})
target_link_libraries(gameserver_lib adminserver_lib)
qt5_use_modules(gameserver_lib Core)
ADD_DEPENDENCIES(gameserver_lib gameData ${target_DEPENDS})
