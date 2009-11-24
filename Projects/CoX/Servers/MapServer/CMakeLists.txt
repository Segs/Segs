# $Id: CMakeLists.txt 306 2007-01-23 12:16:25Z nemerle $
SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/SEGSMap.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/MapClient.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/MapHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/MapServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/Events/NewEntity.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/EntityStorage.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/MapEventFactory.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/MapLink.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/NetCommandManager.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/NetCommandManager.h
${CMAKE_CURRENT_SOURCE_DIR}/include/SEGSMap.h
${CMAKE_CURRENT_SOURCE_DIR}/include/EntityStorage.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapClient.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapLink.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapEvents.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapRef.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapServer.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Events/NewEntity.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(target_DEPENDS "")

SEGS_ADD_COMPONENT(cox_server_common LIB)
SEGS_ADD_COMPONENT(adminserver_lib LIB) # used to access character db

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

ADD_LIBRARY(mapserver_lib STATIC ${target_SOURCES})
ADD_DEPENDENCIES(mapserver_lib ${target_DEPENDS})

SEGS_REGISTER_COMPONENT(mapserver_lib "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )

