SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/EnhancementService.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/EnhancementService.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_library(EnhancementService ${target_SOURCES})
target_link_libraries(EnhancementService PUBLIC Qt5::Core gameData)
target_link_libraries(EnhancementService INTERFACE SEGS_Components )

if(ENABLE_TESTS)
#    add_subdirectory(UnitTests)
endif()
get_target_property(TARGET_SOURCES EnhancementService SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES})
set_target_properties (EnhancementService PROPERTIES
    FOLDER Projects/CoX/Servers/GameServer_Handlers
)
