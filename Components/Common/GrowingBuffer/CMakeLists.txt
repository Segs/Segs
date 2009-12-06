# $Id$
SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/Buffer.cpp 
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/Buffer.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

SEGS_ADD_COMPONENT(Globals SOURCE)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

SEGS_REGISTER_COMPONENT(GrowingBuffer "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )



