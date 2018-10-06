SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/SuperGroupHandler.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/SuperGroupHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/SuperGroupHandlerEvents.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(target_DEPENDS "")

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
scan_event_types(SuperGroupHandlerEvents ${CMAKE_CURRENT_SOURCE_DIR})
add_event_sources(SuperGroupHandlerEvents target_SOURCES)

ADD_LIBRARY(supergroupservice_lib STATIC ${target_SOURCES})
target_link_libraries(supergroupservice_lib PUBLIC Qt5::Core)
target_link_libraries(supergroupservice_lib INTERFACE SEGS_Components network_structures gameData )
ADD_DEPENDENCIES(supergroupservice_lib gameData ${target_DEPENDS})

if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()
