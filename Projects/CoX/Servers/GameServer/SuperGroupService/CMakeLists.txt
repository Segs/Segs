SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/SuperGroupHandler.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/SuperGroupHandler.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_handler(SuperGroupService ${target_SOURCES})
target_link_libraries(SuperGroupService PUBLIC Qt5::Core gameData)
target_link_libraries(SuperGroupService INTERFACE SEGS_Components)

if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES SuperGroupService SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES})
set_target_properties (SuperGroupService PROPERTIES
    FOLDER Projects/CoX/Servers/GameServer_Handlers
)
