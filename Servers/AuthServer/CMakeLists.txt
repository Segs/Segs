SET(target_CPP
    ${CMAKE_CURRENT_SOURCE_DIR}/AuthHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AuthServer.cpp
)
SET(target_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/AuthHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/AuthServer.h
)
SET (authserver_standaloneSRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/AdminRPC.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AdminRPC.h
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(target_DEPENDS "")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
SET (target_SOURCES
  ${target_CPP}
  ${target_INCLUDE}
)
add_handler(Auth ${target_SOURCES})
#FIND_PACKAGE(Boost COMPONENTS system REQUIRED) # Boost_LIBRARIES is not a cache variable, so we  have to do this here again

target_link_libraries(Auth PUBLIC Qt5::Core)
target_link_libraries(Auth PUBLIC ace_IMP cox_server_common AuthProtocol SEGS_Components)
IF(BUILD_SEPARATE)
    ADD_DEFINITIONS(-DSEPARATE)
    ADD_EXECUTABLE(segs_server ${authserver_standaloneSRCS})
    TARGET_LINK_LIBRARIES(segs_server PUBLIC
        Auth
        AuthDatabase
        GameServer
        MapServer
        GameDatabase
        ${target_DEPENDS} gameData
        ${SCRIPTING_LIBRARY}
        Threads::Threads
        ace_IMP
        jcon_IMP
        ${CMAKE_DL_LIBS}
    )
    ADD_DEPENDENCIES(segs_server ${target_DEPENDS} Auth)
    ADD_DEPENDENCIES(segs_server ACE_BUILD)
    target_link_libraries(segs_server PUBLIC Qt5::Core)
    set_property(TARGET segs_server PROPERTY POSITION_INDEPENDENT_CODE ON)
    IF(MINGW)
        target_link_libraries(segs_server PUBLIC ws2_32)
    ENDIF()
    install(TARGETS segs_server
        DESTINATION deploy
    )
ENDIF(BUILD_SEPARATE)

IF(WIN32)
    include(Windeployqt)
    windeployqt(segs_server deploy)
ELSEIF(APPLE)
    include(Macdeployqt)
    macdeployqt(segs_server deploy)
ENDIF()

# Retain directory structure in visual studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${target_CPP} ${target_INCLUDE} ${authserver_standaloneSRCS})
set_target_properties (Auth PROPERTIES
    FOLDER Servers
)
set_target_properties (segs_server PROPERTIES
    FOLDER Servers
)
