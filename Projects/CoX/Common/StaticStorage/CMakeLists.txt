SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/Filesystem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/CostumeStructure.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/MapStructure.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/ReadableStructures.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/CoXHash.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/DataStorage.cpp

)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/Filesystem.h
${CMAKE_CURRENT_SOURCE_DIR}/include/ReadableStructures.h
${CMAKE_CURRENT_SOURCE_DIR}/include/CostumeStructure.h
${CMAKE_CURRENT_SOURCE_DIR}/include/MapStructure.h
${CMAKE_CURRENT_SOURCE_DIR}/include/DataStorage.h
${CMAKE_CURRENT_SOURCE_DIR}/include/CoXHash.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(target_DEPENDS "")

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ACE_path})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

ADD_LIBRARY(static_storage STATIC ${target_SOURCES})
#ADD_DEPENDENCIES(static_storage ${target_DEPENDS})

SEGS_REGISTER_COMPONENT(static_storage "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )
