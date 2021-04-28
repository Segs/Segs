SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/TeamHandler.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/TeamHandler.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_handler(TeamService ${target_SOURCES})
target_link_libraries(TeamService PUBLIC Qt5::Core UserRouterService_interface)
target_link_libraries(TeamService PUBLIC SEGS_Components gameData)

if(ENABLE_TESTS)
#    add_subdirectory(UnitTests)
endif()

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES TeamService SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (TeamService PROPERTIES
    FOLDER Servers/GameServer_Handlers
)
