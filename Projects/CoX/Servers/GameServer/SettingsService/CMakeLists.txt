SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/SettingsService.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/SettingsService.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_handler(SettingsService ${target_SOURCES})
target_link_libraries(SettingsService PUBLIC Qt5::Core)
target_link_libraries(SettingsService PUBLIC SEGS_Components gameData)

if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES SettingsService SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (SettingsService PROPERTIES
    FOLDER Projects/CoX/Servers/GameServer_Handlers
)