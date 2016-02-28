SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/BitStream.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Buffer.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/BitStream.h
${CMAKE_CURRENT_SOURCE_DIR}/Buffer.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
if(ENABLE_SCRIPTING_ENGINE)
    SEGS_ADD_COMPONENT(MRuby LIB)
endif()

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ACE_path})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
add_library(Bitstream ${target_SOURCES} )
add_dependencies(Bitstream Zlib_BUILD)
#add_dependencies(Bitstream ACE_BUILD)
SEGS_REGISTER_COMPONENT(Bitstream "${target_INCLUDE_DIR}" "${target_INCLUDE}" )
