# $Id: CMakeLists.txt 301 2006-12-26 15:50:44Z nemerle $

SET(target_CPP
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/Base.h
${CMAKE_CURRENT_SOURCE_DIR}/include/ConfigExtension.h
${CMAKE_CURRENT_SOURCE_DIR}/include/version.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

SEGS_ADD_COMPONENT(CRUDP_Protocol SOURCE)
INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

#MESSAGE(ERROR_FATAL " FILES ${target_SOURCES}")

ADD_LIBRARY(segs_common STATIC ${target_SOURCES})
