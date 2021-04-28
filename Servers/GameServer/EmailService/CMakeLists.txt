SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/EmailHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/EmailService.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/EmailHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/EmailService.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
add_handler(EmailService ${target_SOURCES})
target_link_libraries(EmailService PUBLIC Qt5::Core gameData)
target_link_libraries(EmailService INTERFACE SEGS_Components )

if(ENABLE_TESTS)
#    add_subdirectory(UnitTests)
endif()
get_target_property(TARGET_SOURCES EmailService SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (EmailService PROPERTIES
    FOLDER Servers/GameServer_Handlers
)
