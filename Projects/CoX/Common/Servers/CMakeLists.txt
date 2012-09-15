# $Id$

SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/AdminServerInterface.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthServerInterface.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/GameServerInterface.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/MapServerInterface.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/ServerSupport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/RoamingServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/ServerHandle.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/ServerManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/InterfaceManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/Database.cpp
#${CMAKE_CURRENT_SOURCE_DIR}/src/CRUD_Link.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/AdminServerInterface.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthServerInterface.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Client.h
${CMAKE_CURRENT_SOURCE_DIR}/include/GameServerInterface.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapServerInterface.h
${CMAKE_CURRENT_SOURCE_DIR}/include/ServerHandle.h
${CMAKE_CURRENT_SOURCE_DIR}/include/server_support.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Server.h
${CMAKE_CURRENT_SOURCE_DIR}/include/RoamingServer.h
${CMAKE_CURRENT_SOURCE_DIR}/include/InterfaceManager.h
${CMAKE_CURRENT_SOURCE_DIR}/include/ServerManager.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Database.h
${CMAKE_CURRENT_SOURCE_DIR}/include/CRUD_Link.h
${CMAKE_CURRENT_SOURCE_DIR}/include/CRUD_Events.h
${CMAKE_CURRENT_SOURCE_DIR}/include/crud_link.inl
)

FIND_PACKAGE(POSTGRE REQUIRED)
FIND_PACKAGE(ACE REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(target_DEPENDS "")

SEGS_ADD_COMPONENT(segs_common LIB)
SEGS_ADD_COMPONENT(EventSystem LIB)
SEGS_ADD_COMPONENT(network_structures LIB)

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
