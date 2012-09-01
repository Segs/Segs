
# $Id$
SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthEventFactory.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthPacketCodec.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/Events/ServerListResponse.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthEvents.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthEventFactory.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthOpcodes.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthPacketCodec.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Events/AuthorizationProtocolVersion.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Events/AuthorizationError.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Events/DbError.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Events/ServerSelectRequest.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Events/ServerSelectResponse.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Events/LoginResponse.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Events/LoginRequest.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Events/ServerListResponse.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Events/ServerListRequest.h
)
set(Boost_ADDITIONAL_VERSIONS "1.51" "1.51.0")
FIND_PACKAGE(Boost COMPONENTS system REQUIRED)

SET(target_INCLUDE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Events"
        ${Boost_INCLUDE_DIRS}
)
# add components here
SEGS_ADD_COMPONENT(EventSystem LIB)
# end of additional components

INCLUDE_DIRECTORIES(
${target_INCLUDE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/include/Events/
)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

#ADD_LIBRARY(AuthProtocol STATIC ${target_SOURCES})

SEGS_REGISTER_COMPONENT(AuthProtocol "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )
