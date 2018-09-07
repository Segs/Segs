SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/FriendHandler.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/FriendHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/FriendHandlerEvents.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(target_DEPENDS "")

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
scan_event_types(FriendHandlerEvents ${CMAKE_CURRENT_SOURCE_DIR})
add_event_sources(FriendHandlerEvents target_SOURCES)

ADD_LIBRARY(friendshipservice_lib STATIC ${target_SOURCES})
target_link_libraries(friendshipservice_lib PUBLIC Qt5::Core)
target_link_libraries(friendshipservice_lib INTERFACE SEGS_Components network_structures gameData )
ADD_DEPENDENCIES(friendshipservice_lib gameData ${target_DEPENDS})

if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()
