# $Id$
SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/Auth.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthFSM.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthConnection_ClientSide.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/Auth.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthFSM.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthConnection_ClientSide.h
)
SET(target_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

SEGS_ADD_COMPONENT(GrowingBuffer SOURCE)
SEGS_ADD_COMPONENT(AuthProtocol SOURCE)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

#ADD_LIBRARY(Auth STATIC ${target_SOURCES})
SEGS_REGISTER_COMPONENT(Auth "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )
