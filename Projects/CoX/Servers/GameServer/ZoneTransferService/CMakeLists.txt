SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/ZoneTransferService.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/ZoneTransferService.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_library(ZoneTransferService ${target_SOURCES})
target_link_libraries(ZoneTransferService PUBLIC Qt5::Core gameData)
target_link_libraries(ZoneTransferService INTERFACE SEGS_Components )

if(ENABLE_TESTS)
#    add_subdirectory(UnitTests)
endif()
get_target_property(TARGET_SOURCES ZoneTransferService SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES})
set_target_properties (ZoneTransferService PROPERTIES
    FOLDER Projects/CoX/Servers/GameServer_Handlers
)
