
SET(target_CPP
    ${CMAKE_CURRENT_SOURCE_DIR}/AdminServerInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AuthServerInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GameServerInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MapServerInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ServerSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RoamingServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ServerEndpoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ServerHandle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ServerManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterfaceManager.cpp
)
SET(target_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/AdminServerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/AuthServerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ClientManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GameServerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MapServerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ServerEndpoint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ServerHandle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ServerProtocolHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/server_support.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Server.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RoamingServer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/InterfaceManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ServerManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Database.h
    ${CMAKE_CURRENT_SOURCE_DIR}/InternalEvents.h
    ${CMAKE_CURRENT_SOURCE_DIR}/opcodes/ControlCodes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/opcodes/NameTables.h
    ${CMAKE_CURRENT_SOURCE_DIR}/opcodes/Opcodes.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(target_DEPENDS ace_IMP)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ACE_path})

ADD_LIBRARY(cox_server_common STATIC ${target_SOURCES})
add_dependencies(cox_server_common ace_IMP)
target_link_libraries(cox_server_common INTERFACE ace_IMP)
set_target_properties(cox_server_common PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${target_INCLUDE_DIR})
qt5_use_modules(cox_server_common LINK_PUBLIC Core Sql)
