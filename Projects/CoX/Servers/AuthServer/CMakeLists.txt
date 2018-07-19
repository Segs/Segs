SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/AuthHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuthServer.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/AuthHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthServer.h
)
SET (authserver_standaloneSRCS
${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(target_DEPENDS "")

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
SET (target_SOURCES
  ${target_CPP}
  ${target_INCLUDE}
)

#FIND_PACKAGE(Boost COMPONENTS system REQUIRED) # Boost_LIBRARIES is not a cache variable, so we  have to do this here again

ADD_LIBRARY(authserver_lib STATIC ${target_SOURCES})
target_link_libraries(authserver_lib PUBLIC Qt5::Core)
add_dependencies(authserver_lib ace_IMP)
IF(BUILD_SEPARATE)
    ADD_DEFINITIONS(-DSEPARATE)
    ADD_EXECUTABLE(authserver ${authserver_standaloneSRCS})
    TARGET_LINK_LIBRARIES(authserver PUBLIC
        authserver_lib
        auth_db_sync_lib
        gameserver_lib
        mapserver_lib
        game_db_sync_lib
        cox_server_common
        network_structures
        AuthProtocol
        ${target_DEPENDS} gameData
        SEGS_Components
        ${SCRIPTING_LIBRARY}
        Threads::Threads
        ace_IMP
        ${CMAKE_DL_LIBS}
        password_hasher
    )
    ADD_DEPENDENCIES(authserver ${target_DEPENDS} authserver_lib)
    ADD_DEPENDENCIES(authserver ACE_BUILD)
    target_link_libraries(authserver PUBLIC Qt5::Core)
    set_property(TARGET authserver PROPERTY POSITION_INDEPENDENT_CODE ON)
    IF(MINGW)
        target_link_libraries(authserver PUBLIC ws2_32)
    ENDIF()
    install(TARGETS authserver
        DESTINATION ${CPACK_BIN_DIR}
    )
ENDIF(BUILD_SEPARATE)

IF(WIN32)
    include(Windeployqt)
    windeployqt(authserver deploy)
ENDIF()
