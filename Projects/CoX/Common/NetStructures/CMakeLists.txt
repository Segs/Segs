# $Id: CMakeLists.txt 301 2006-12-26 15:50:44Z nemerle $

SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/Character.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/CommonNetStructures.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/Costume.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/Entity.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/CommonNetStructures.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Powers.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Character.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Costume.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Entity.h
)

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
INCLUDE_DIRECTORIES(${ACE_path})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

ADD_LIBRARY(network_structures STATIC ${target_SOURCES})
ADD_DEPENDENCIES(network_structures ${target_DEPENDS})

SEGS_REGISTER_COMPONENT(network_structures "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )
