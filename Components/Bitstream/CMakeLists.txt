# $Id: CMakeLists.txt 301 2006-12-26 15:50:44Z nemerle $
SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/Bitstream.cpp 
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/Bitstream.h
)


SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

SEGS_ADD_COMPONENT(GrowingBuffer SOURCE)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

SEGS_REGISTER_COMPONENT(Bitstream "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )


