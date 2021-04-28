SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/ClientOptionService.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/ClientOptionService.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_handler(ClientOptionService ${target_SOURCES})
add_messages_library(ClientOptionService ${SOURCES})
target_link_libraries(ClientOptionService PUBLIC Qt5::Core gameData)
target_link_libraries(ClientOptionService INTERFACE SEGS_Components )

if(ENABLE_TESTS)
#    add_subdirectory(UnitTests)
endif()
get_target_property(TARGET_SOURCES ClientOptionService SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (ClientOptionService PROPERTIES
    FOLDER Servers/GameServer_Handlers
)
