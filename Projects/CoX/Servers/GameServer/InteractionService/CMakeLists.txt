SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/InteractionService.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/InteractionService.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_library(InteractionService ${target_SOURCES})
target_link_libraries(InteractionService PUBLIC Qt5::Core gameData)
target_link_libraries(InteractionService INTERFACE SEGS_Components )

if(ENABLE_TESTS)
#    add_subdirectory(UnitTests)
endif()
get_target_property(TARGET_SOURCES InteractionService SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES})
set_target_properties (InteractionService PROPERTIES
    FOLDER Projects/CoX/Servers/GameServer_Handlers
)
