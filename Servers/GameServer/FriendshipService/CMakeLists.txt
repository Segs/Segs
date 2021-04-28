SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/FriendHandler.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/FriendHandler.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_handler(FriendshipService ${target_SOURCES})
target_link_libraries(FriendshipService PUBLIC Qt5::Core)
target_link_libraries(FriendshipService PUBLIC SEGS_Components gameData)

if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES FriendshipService SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (FriendshipService PROPERTIES
    FOLDER Servers/GameServer_Handlers
)