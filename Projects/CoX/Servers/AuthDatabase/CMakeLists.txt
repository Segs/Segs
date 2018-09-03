SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/AuthDBSync.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuthDBSyncHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuthDBSyncContext.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/AuthDBSync.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthDBSyncHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthDBSyncEvents.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthDBSyncContext.h
)
SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
scan_event_types(AuthDBEvents ${CMAKE_CURRENT_SOURCE_DIR})
add_event_sources(AuthDBEvents target_SOURCES)
ADD_LIBRARY(auth_db_sync_lib STATIC ${target_SOURCES})
target_link_libraries(auth_db_sync_lib PUBLIC cox_server_common network_structures Qt5::Core Qt5::Sql)
target_link_libraries(auth_db_sync_lib INTERFACE SEGS_Components)
target_link_libraries(auth_db_sync_lib PRIVATE cereal_IMP)
target_include_directories(auth_db_sync_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()
