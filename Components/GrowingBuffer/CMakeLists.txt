# $Id: CMakeLists.txt 301 2006-12-26 15:50:44Z nemerle $
SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/Buffer.cpp 
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/Buffer.h
)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

SEGS_REGISTER_COMPONENT(GrowingBuffer "${target_INCLUDE_DIR}" ${target_CPP} ${target_INCLUDE} )

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

ADD_LIBRARY(growing_buffer STATIC ${target_SOURCES})



