SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/GameHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameEventFactory.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameLink.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/GameServer.h
${CMAKE_CURRENT_SOURCE_DIR}/GameHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/GameEventFactory.h
${CMAKE_CURRENT_SOURCE_DIR}/GameLink.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
    ${target_CPP}
    ${target_INCLUDE}
)

add_handler(GameServer ${target_SOURCES})

target_link_libraries(GameServer PUBLIC Qt5::Core)
target_link_libraries(GameServer INTERFACE SEGS_Components gameData )

add_subdirectory(FriendshipService)
add_subdirectory(EmailService)
add_subdirectory(ClientOptionService)
add_subdirectory(TeamService)

target_link_libraries(GameServer PUBLIC Qt5::Core FriendshipService EmailService TeamService ClientOptionService )
target_link_libraries(GameServer INTERFACE SEGS_Components gameData)

if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()

get_target_property(TARGET_SOURCES GameServer SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (GameServer PROPERTIES
    FOLDER Servers
)
