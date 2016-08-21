
SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/AdminServerInterface.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuthServerInterface.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GameServerInterface.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MapServerInterface.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
${CMAKE_CURRENT_SOURCE_DIR}/ServerSupport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/RoamingServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/ServerHandle.cpp
${CMAKE_CURRENT_SOURCE_DIR}/ServerManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/InterfaceManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Database.cpp
#${CMAKE_CURRENT_SOURCE_DIR}/CRUD_Link.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/AdminServerInterface.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthServerInterface.h
${CMAKE_CURRENT_SOURCE_DIR}/Client.h
${CMAKE_CURRENT_SOURCE_DIR}/GameServerInterface.h
${CMAKE_CURRENT_SOURCE_DIR}/MapServerInterface.h
${CMAKE_CURRENT_SOURCE_DIR}/ServerHandle.h
${CMAKE_CURRENT_SOURCE_DIR}/server_support.h
${CMAKE_CURRENT_SOURCE_DIR}/Server.h
${CMAKE_CURRENT_SOURCE_DIR}/RoamingServer.h
${CMAKE_CURRENT_SOURCE_DIR}/InterfaceManager.h
${CMAKE_CURRENT_SOURCE_DIR}/ServerManager.h
${CMAKE_CURRENT_SOURCE_DIR}/Database.h
${CMAKE_CURRENT_SOURCE_DIR}/CRUD_Link.h
${CMAKE_CURRENT_SOURCE_DIR}/CRUD_Events.h
${CMAKE_CURRENT_SOURCE_DIR}/crud_link.inl
)

if(PostgreSQL_FOUND)
    SET(PsqlDbPlugin_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/PsqlDatabase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/PsqlDatabase.h
    )
endif()

SET(SqliteDbPlugin_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/SqliteDatabase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SqliteDatabase.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(target_DEPENDS "")

SEGS_ADD_COMPONENT(ConfigExtensions LIB)
SEGS_ADD_COMPONENT(EventSystem LIB)
SEGS_ADD_COMPONENT(network_structures LIB)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
${PsqlDbPlugin_SOURCES}
${SqliteDbPlugin_SOURCES}
)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PostgreSQL_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${SQLite_path})
INCLUDE_DIRECTORIES(${ACE_path})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

ADD_LIBRARY(cox_server_common STATIC ${target_SOURCES})
ADD_DEPENDENCIES(cox_server_common ${target_DEPENDS})
qt5_use_modules(cox_server_common Core)
SEGS_REGISTER_COMPONENT(cox_server_common "${target_INCLUDE_DIR}" "${target_INCLUDE}" )
