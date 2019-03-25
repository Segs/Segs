SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/ChatService.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/ChatService.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_library(ChatService ${target_SOURCES})
target_link_libraries(ChatService PUBLIC Qt5::Core gameData)
target_link_libraries(ChatService INTERFACE SEGS_Components )

if(ENABLE_TESTS)
#    add_subdirectory(UnitTests)
endif()
get_target_property(TARGET_SOURCES ChatService SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES})
set_target_properties (ChatService PROPERTIES
    FOLDER Projects/CoX/Servers/GameServer_Handlers
)
