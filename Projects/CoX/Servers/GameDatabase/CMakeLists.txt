SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/GameDBSync.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameDBSyncHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameDBSyncContext.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameDBSyncService.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/GameDBSync.h
${CMAKE_CURRENT_SOURCE_DIR}/GameDBSyncHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/GameDBSyncEvents.h
${CMAKE_CURRENT_SOURCE_DIR}/GameDBSyncContext.h
${CMAKE_CURRENT_SOURCE_DIR}/GameDBSyncService.h
)
SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
scan_event_types(GameDBEvents ${CMAKE_CURRENT_SOURCE_DIR})
add_event_sources(GameDBEvents target_SOURCES)
ADD_LIBRARY(game_db_sync_lib STATIC ${target_SOURCES})
target_link_libraries(game_db_sync_lib PUBLIC cox_server_common network_structures Qt5::Core Qt5::Sql)
target_link_libraries(game_db_sync_lib PRIVATE cereal_IMP)
target_link_libraries(game_db_sync_lib INTERFACE SEGS_Components gameData)
target_include_directories(game_db_sync_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()
