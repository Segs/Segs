set(SOURCES
    AuthEvents.h
    AuthorizationError.h
    AuthorizationProtocolVersion.h
    DbError.h
    LoginRequest.h
    LoginResponse.h
    ServerListRequest.h
    ServerListResponse.cpp
    ServerListResponse.h
    ServerSelectRequest.h
    ServerSelectResponse.h
)
add_messages_library(Auth ${SOURCES})
