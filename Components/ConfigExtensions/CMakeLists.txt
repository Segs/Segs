SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/ConfigExtension.cpp

)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/ConfigExtension.h
)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_library(ConfigExtensions ${target_SOURCES})
SEGS_REGISTER_COMPONENT(ConfigExtensions "${target_INCLUDE_DIR}" "${target_INCLUDE}" )

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})




