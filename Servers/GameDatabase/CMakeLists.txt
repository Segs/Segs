SET(target_CPP
    ${CMAKE_CURRENT_SOURCE_DIR}/GameDBSync.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GameDBSyncHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GameDBSyncContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GameDBSyncService.cpp
)
SET(target_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/GameDBSync.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GameDBSyncHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GameDBSyncContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GameDBSyncService.h
)
SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_handler(GameDatabase ${target_SOURCES})
target_link_libraries(GameDatabase PUBLIC cox_server_common gameData Qt5::Core Qt5::Sql)
target_link_libraries(GameDatabase PRIVATE cereal_IMP)
target_link_libraries(GameDatabase INTERFACE SEGS_Components)

if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()
# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES GameDatabase SOURCES) 
list(REMOVE_ITEM TARGET_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/GameDBEvents.gen.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (GameDatabase PROPERTIES
    FOLDER Servers
)
