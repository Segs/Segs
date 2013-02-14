SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/HashStorage.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/CharacterClient.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/GameHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/GameServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/GameEvents.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/GameEventFactory.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/GameLink.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/HashStorage.h
${CMAKE_CURRENT_SOURCE_DIR}/include/CharacterClient.h
${CMAKE_CURRENT_SOURCE_DIR}/include/GameServer.h
${CMAKE_CURRENT_SOURCE_DIR}/include/GameHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/include/GameEventFactory.h
${CMAKE_CURRENT_SOURCE_DIR}/include/GameEvents.h
${CMAKE_CURRENT_SOURCE_DIR}/include/GameLink.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(target_DEPENDS "")

SEGS_ADD_COMPONENT(cox_server_common LIB)
SEGS_ADD_COMPONENT(adminserver_lib LIB) # later on this will be an AdminServerProxy component

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
ADD_LIBRARY(gameserver_lib STATIC ${target_SOURCES})
ADD_DEPENDENCIES(gameserver_lib ${target_DEPENDS})

SEGS_REGISTER_COMPONENT(gameserver_lib "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )
