SET(target_CPP
    CRUDP_Packet.cpp
    CRUDP_Protocol.cpp
    CRUD_Link.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/GameProtocol.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/GameProtocolHandler.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/PacketBase.cpp
    PacketCodec.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/CRUDP_Packet.h
${CMAKE_CURRENT_SOURCE_DIR}/CRUDP_Protocol.h
#${CMAKE_CURRENT_SOURCE_DIR}/GameProtocol.h
#${CMAKE_CURRENT_SOURCE_DIR}/GameProtocolHandler.h
#${CMAKE_CURRENT_SOURCE_DIR}/PacketBase.h
${CMAKE_CURRENT_SOURCE_DIR}/PacketCodec.h
${CMAKE_CURRENT_SOURCE_DIR}/CRUD_Events.h
${CMAKE_CURRENT_SOURCE_DIR}/CRUD_Link.h
${CMAKE_CURRENT_SOURCE_DIR}/ILink.h
)


SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ACE_path})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

ADD_LIBRARY(CRUDP_Protocol STATIC ${target_SOURCES})
target_include_directories(CRUDP_Protocol INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

qt5_use_modules(CRUDP_Protocol LINK_PUBLIC Core)
add_dependencies(CRUDP_Protocol ace_IMP)


