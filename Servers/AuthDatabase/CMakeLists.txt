SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/AuthDBSync.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuthDBSyncHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuthDBSyncContext.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/AuthDBSync.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthDBSyncHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthDBSyncContext.h
)
SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_handler(AuthDatabase ${target_SOURCES})
target_link_libraries(AuthDatabase PUBLIC cox_server_common gameData Qt5::Core Qt5::Sql)
target_link_libraries(AuthDatabase INTERFACE SEGS_Components)
target_link_libraries(AuthDatabase PRIVATE cereal_IMP)

if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()
# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES AuthDatabase SOURCES) 
list(REMOVE_ITEM TARGET_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/AuthDBEvents.gen.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (AuthDatabase PROPERTIES
    FOLDER Servers
)
