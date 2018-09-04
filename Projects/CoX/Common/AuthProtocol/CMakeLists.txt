SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/AuthEventFactory.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuthLink.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuthPacketCodec.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Events/ServerListResponse.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/AuthEvents.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthEventFactory.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthLink.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthOpcodes.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthPacketCodec.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/AuthorizationProtocolVersion.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/AuthorizationError.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/DbError.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/ServerSelectRequest.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/ServerSelectResponse.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/LoginResponse.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/LoginRequest.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/ServerListResponse.h
${CMAKE_CURRENT_SOURCE_DIR}/Events/ServerListRequest.h
)

SET(target_INCLUDE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/Events"
)

# add components here
# end of additional components

INCLUDE_DIRECTORIES(
${target_INCLUDE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}Events/
)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
scan_event_types(AuthEvents ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Events)
add_event_sources(AuthEvents target_SOURCES)

ADD_LIBRARY(AuthProtocol STATIC ${target_SOURCES})
target_link_libraries(AuthProtocol PUBLIC Qt5::Core)
target_link_libraries(AuthProtocol INTERFACE SEGS_Components)
set_property(TARGET AuthProtocol PROPERTY POSITION_INDEPENDENT_CODE ON)
if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()
