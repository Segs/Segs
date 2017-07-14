SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/Character.cpp
${CMAKE_CURRENT_SOURCE_DIR}/CommonNetStructures.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Costume.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Powers.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Entity.cpp
${CMAKE_CURRENT_SOURCE_DIR}/StateInterpolator.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/CommonNetStructures.h
${CMAKE_CURRENT_SOURCE_DIR}/Powers.h
${CMAKE_CURRENT_SOURCE_DIR}/Character.h
${CMAKE_CURRENT_SOURCE_DIR}/Costume.h
${CMAKE_CURRENT_SOURCE_DIR}/Entity.h
)


SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(target_DEPENDS "")

SEGS_ADD_COMPONENT(CRUDP_Protocol LIB)
SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ACE_path})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

ADD_LIBRARY(network_structures STATIC ${target_SOURCES})
qt5_use_modules(network_structures Core)
ADD_DEPENDENCIES(network_structures ${target_DEPENDS} cereal_IMP CRUDP_Protocol)

SEGS_REGISTER_COMPONENT(network_structures "${target_INCLUDE_DIR}" "${target_INCLUDE}" )
