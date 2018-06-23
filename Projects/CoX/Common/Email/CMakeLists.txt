INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/EmailHandler.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/EmailHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/EmailEvents.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

ADD_LIBRARY(email_handler STATIC ${target_SOURCES})
set_target_properties(email_handler PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(email_handler PUBLIC Qt5::Core)
ADD_DEPENDENCIES(email_handler cereal_IMP CRUDP_Protocol)
