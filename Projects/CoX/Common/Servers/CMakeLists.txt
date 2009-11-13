# $Id: CMakeLists.txt 301 2006-12-26 15:50:44Z nemerle $

SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/ServerProtocolHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/AdminServerInterface.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthServerInterface.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/GameServerInterface.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/MapServerInterface.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/NetManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/ServerSupport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/RoamingServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/ServerEndpoint.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/ServerHandle.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/ServerManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/InterfaceManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/Database.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/AdminServerInterface.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthServerInterface.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Client.h
${CMAKE_CURRENT_SOURCE_DIR}/include/CommonNetStructures.h
${CMAKE_CURRENT_SOURCE_DIR}/include/GameServerInterface.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapServerInterface.h
${CMAKE_CURRENT_SOURCE_DIR}/include/NetManager.h
${CMAKE_CURRENT_SOURCE_DIR}/include/ServerHandle.h
${CMAKE_CURRENT_SOURCE_DIR}/include/server_support.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Server.h
${CMAKE_CURRENT_SOURCE_DIR}/include/RoamingServer.h
${CMAKE_CURRENT_SOURCE_DIR}/include/InterfaceManager.h
${CMAKE_CURRENT_SOURCE_DIR}/include/ServerManager.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Database.h
)

FIND_PACKAGE(POSTGRE REQUIRED)
FIND_PACKAGE(ACE REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(target_DEPENDS "")

SEGS_ADD_COMPONENT(segs_common LIB)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PGSQL_path})
INCLUDE_DIRECTORIES(${ACE_path})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

ADD_LIBRARY(cox_server_common STATIC ${target_SOURCES})
ADD_DEPENDENCIES(cox_server_common ${target_DEPENDS})

SEGS_REGISTER_COMPONENT(cox_server_common "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )
