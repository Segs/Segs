SET(target_CPP
    CRUDP_Packet.cpp
    CRUDP_Protocol.cpp
    CRUD_Events.cpp
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
scan_event_types(CRUDPEvents ${CMAKE_CURRENT_SOURCE_DIR})
add_event_sources(CRUDPEvents target_SOURCES)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${ACE_path})

ADD_LIBRARY(CRUDP_Protocol STATIC ${target_SOURCES})
target_include_directories(CRUDP_Protocol INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(CRUDP_Protocol PUBLIC Qt5::Core)
target_link_libraries(CRUDP_Protocol INTERFACE SEGS_Components)
add_dependencies(CRUDP_Protocol ace_IMP)


if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES CRUDP_Protocol SOURCES) 
list(REMOVE_ITEM TARGET_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/CRUDPEvents.gen.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (CRUDP_Protocol PROPERTIES
    FOLDER Common
)
