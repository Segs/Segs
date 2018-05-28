
SET(target_CPP
    ${CMAKE_CURRENT_SOURCE_DIR}/ServerEndpoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HandlerLocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MessageBus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MessageBusEndpoint.cpp
)
SET(target_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/ClientManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ServerEndpoint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Database.h
    ${CMAKE_CURRENT_SOURCE_DIR}/InternalEvents.h
    ${CMAKE_CURRENT_SOURCE_DIR}/HandlerLocator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MessageBus.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MessageBusEndpoint.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(target_DEPENDS ace_IMP)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${ACE_path})

ADD_LIBRARY(cox_server_common STATIC ${target_SOURCES})
add_dependencies(cox_server_common ace_IMP)
target_link_libraries(cox_server_common INTERFACE ace_IMP)
target_link_libraries(cox_server_common PUBLIC Qt5::Core Qt5::Sql)
set_target_properties(cox_server_common PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${target_INCLUDE_DIR})
