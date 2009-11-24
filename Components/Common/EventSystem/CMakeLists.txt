# $Id: CMakeLists.txt 301 2006-12-26 15:50:44Z nemerle $
SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/EventProcessor.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/EventProcessor.h
${CMAKE_CURRENT_SOURCE_DIR}/include/SEGSEvent.h
${CMAKE_CURRENT_SOURCE_DIR}/include/LinkLevelEvent.h
)

FIND_PACKAGE(ACE REQUIRED)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

SEGS_ADD_COMPONENT(Globals SOURCE)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)


INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ACE_path})

ADD_LIBRARY(EventSystem STATIC ${target_SOURCES})

SEGS_REGISTER_COMPONENT(EventSystem "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )


