SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/GameHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameEvents.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameEventFactory.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameLink.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/GameServer.h
${CMAKE_CURRENT_SOURCE_DIR}/GameHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/GameEventFactory.h
${CMAKE_CURRENT_SOURCE_DIR}/GameEvents.h
${CMAKE_CURRENT_SOURCE_DIR}/GameLink.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(target_DEPENDS "")

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
scan_event_types(GameEvents ${CMAKE_CURRENT_SOURCE_DIR})
add_event_sources(GameEvents target_SOURCES)

add_subdirectory(FriendshipService)
ADD_LIBRARY(gameserver_lib STATIC ${target_SOURCES})
target_link_libraries(gameserver_lib PUBLIC Qt5::Core friendshipservice_lib )
target_link_libraries(gameserver_lib INTERFACE SEGS_Components network_structures gameData )
ADD_DEPENDENCIES(gameserver_lib gameData ${target_DEPENDS})
if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()
