# $Id: CMakeLists.txt 306 2007-01-23 12:16:25Z nemerle $
SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/SEGSMap.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/MapClient.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/MapHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/MapServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/MapServerEndpoint.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/MapPacket.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/Entity.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/EntityStorage.cpp
#../../Common/src/RoamingServer.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/SEGSMap.h
${CMAKE_CURRENT_SOURCE_DIR}/include/EntityStorage.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapClient.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapPacket.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapServer.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapServerEndpoint.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapHandler.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(target_DEPENDS "")

SEGS_ADD_COMPONENT(segs_server_common LIB)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

ADD_LIBRARY(mapserver_lib STATIC ${target_SOURCES})
ADD_DEPENDENCIES(mapserver_lib ${target_DEPENDS})

SEGS_REGISTER_COMPONENT(mapserver_lib "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )