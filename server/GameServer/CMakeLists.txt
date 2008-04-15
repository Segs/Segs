# $Id: CMakeLists.txt 294 2006-10-12 17:52:28Z nemerle $
SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/Character.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/CharacterClient.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/CharacterDatabase.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/ClientHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/GamePacket.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/GameServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/GameServerEndpoint.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/Character.h
${CMAKE_CURRENT_SOURCE_DIR}/include/CharacterClient.h
${CMAKE_CURRENT_SOURCE_DIR}/include/CharacterDatabase.h
${CMAKE_CURRENT_SOURCE_DIR}/include/ClientHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/include/GameServer.h
${CMAKE_CURRENT_SOURCE_DIR}/include/GamePacket.h
${CMAKE_CURRENT_SOURCE_DIR}/include/GameServerEndpoint.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(target_DEPENDS "")

SEGS_ADD_COMPONENT(segs_common LIB)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
ADD_LIBRARY(gameserver_lib STATIC ${target_SOURCES})
ADD_DEPENDENCIES(gameserver_lib ${target_DEPENDS})

SEGS_REGISTER_COMPONENT(gameserver_lib "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )
