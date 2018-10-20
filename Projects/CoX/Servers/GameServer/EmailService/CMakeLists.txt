SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/EmailHandler.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/EmailHandler.h
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
