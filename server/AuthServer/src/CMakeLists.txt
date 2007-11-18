# $Id: CMakeLists.txt 316 2007-01-25 15:17:16Z nemerle $

SET (authserver_SRCS 
AuthClient.cpp
AuthClientServicer.cpp 
AuthFSM.cpp 
AuthPacket.cpp 
AuthPacketCodec.cpp 
AuthProtocol.cpp 
AuthServer.cpp 
ClientConnection.cpp
packets/authserverlist.cpp
../include/AuthClient.h
../include/AuthClientServicer.h
../include/AuthFSM.h 
../include/AuthPacket.h
../include/AuthPacketCodec.h
../include/AuthServer.h
../include/ClientConnection.h
../include/AuthProtocol.h
../include/Packets/authversion.h
../include/Packets/authlogin.h
../include/Packets/authloginresponse.h
../include/Packets/authrequestserverlist.h
../include/Packets/authserverlist.h
../include/Packets/authselectserver.h
../include/Packets/authselectserverresponse.h
)

SET (authserver_standaloneSRCS 
main.cpp)

SOURCE_GROUP("Source Files\\packets" FILES packets/authserverlist.cpp)

ADD_LIBRARY(authserver_lib STATIC ${authserver_SRCS})

IF(BUILD_SEPARATE)
	ADD_DEFINITIONS(-DSEPARATE)
	ADD_EXECUTABLE(authserver ${authserver_standaloneSRCS})
	TARGET_LINK_LIBRARIES(authserver segs_common segs_server_common authserver_lib adminserver_lib gameserver_lib mapserver_lib 
						${ACE_lib} 
						${PGSQL_lib} 
						${ZLIB_LIBRARIES}
						${CMAKE_THREAD_LIBS_INIT})
	ADD_DEPENDENCIES(authserver authserver_lib segs_common gameserver_lib)
ENDIF(BUILD_SEPARATE)
