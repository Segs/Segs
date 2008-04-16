# $Id: CMakeLists.txt 316 2007-01-25 15:17:16Z nemerle $

SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthClient.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthClientServicer.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthConnection_ServerSide.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthFSM.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthServer.cpp 
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthClient.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthClientServicer.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthFSM.h 
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthServer.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthConnection_ServerSide.h
)
SET (authserver_standaloneSRCS 
${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(target_DEPENDS "")
SEGS_ADD_COMPONENT(AuthProtocol SOURCE)
SEGS_ADD_COMPONENT(segs_server_common LIB)
SEGS_ADD_COMPONENT(adminserver_lib LIB)
SEGS_ADD_COMPONENT(gameserver_lib LIB)
SEGS_ADD_COMPONENT(mapserver_lib LIB)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

ADD_LIBRARY(authserver_lib STATIC ${target_SOURCES})
IF(BUILD_SEPARATE)
	ADD_DEFINITIONS(-DSEPARATE)
	ADD_EXECUTABLE(authserver ${authserver_standaloneSRCS})
    TARGET_LINK_LIBRARIES(authserver   ${target_DEPENDS} authserver_lib
                    						${ACE_lib} 
                    						${PGSQL_lib} 
                    						${ZLIB_LIBRARIES}
                    						${CMAKE_THREAD_LIBS_INIT})
    ADD_DEPENDENCIES(authserver ${target_DEPENDS} authserver_lib)
ENDIF(BUILD_SEPARATE)

SEGS_REGISTER_COMPONENT(authserver_lib "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )