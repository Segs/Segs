SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/CharacterService.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/CharacterService.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_library(CharacterService ${target_SOURCES})
target_link_libraries(CharacterService PUBLIC Qt5::Core gameData)
target_link_libraries(CharacterService INTERFACE SEGS_Components )

add_subdirectory(EnhancementService)
add_subdirectory(InspirationService)
add_subdirectory(PowerService)

target_link_libraries(CharacterService PUBLIC Qt5::Core EnhancementService InspirationService PowerService)

if(ENABLE_TESTS)
#    add_subdirectory(UnitTests)
endif()
get_target_property(TARGET_SOURCES CharacterService SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES})
set_target_properties (CharacterService PROPERTIES
    FOLDER Projects/CoX/Servers/GameServer_Handlers
)
