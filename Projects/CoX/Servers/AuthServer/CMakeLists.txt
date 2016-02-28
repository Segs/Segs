SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/AuthClient.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuthLink.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuthHandler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuthServer.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/AuthClient.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthLink.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthHandler.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthServer.h
)
SET (authserver_standaloneSRCS
${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(target_DEPENDS "")

SEGS_ADD_COMPONENT(AuthProtocol LIB)
SEGS_ADD_COMPONENT(cox_server_common LIB)
SEGS_ADD_COMPONENT(static_storage LIB)
SEGS_ADD_COMPONENT(adminserver_lib LIB)
SEGS_ADD_COMPONENT(gameserver_lib LIB)
SEGS_ADD_COMPONENT(mapserver_lib LIB)
if(ENABLE_SCRIPTING_ENGINE)
    SEGS_ADD_COMPONENT(MRuby LIB)
endif()

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
  ${target_CPP}
  ${target_INCLUDE}
)

#FIND_PACKAGE(Boost COMPONENTS system REQUIRED) # Boost_LIBRARIES is not a cache variable, so we  have to do this here again

ADD_LIBRARY(authserver_lib STATIC ${target_SOURCES})

IF(BUILD_SEPARATE)
    ADD_DEFINITIONS(-DSEPARATE)
    ADD_EXECUTABLE(authserver ${authserver_standaloneSRCS})
    TARGET_LINK_LIBRARIES(authserver
        authserver_lib ${target_DEPENDS}
        ${MRuby_LIBRARY}
        ${ZLIB_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${ACE_LIBRARIES}
        ${PostgreSQL_LIBRARIES} ${OPENSCENEGRAPH_LIBRARIES}
        ${CMAKE_DL_LIBS}
    )
    ADD_DEPENDENCIES(authserver ${target_DEPENDS} authserver_lib)
    ADD_DEPENDENCIES(authserver ACE_BUILD Zlib_BUILD OSG_BUILD)
ENDIF(BUILD_SEPARATE)

SEGS_REGISTER_COMPONENT(authserver_lib "${target_INCLUDE_DIR}" "${target_INCLUDE}" )
