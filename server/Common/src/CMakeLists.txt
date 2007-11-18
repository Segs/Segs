# $Id: CMakeLists.txt 301 2006-12-26 15:50:44Z nemerle $

SET (segs_server_common_SRCS 
AdminServerInterface.cpp
AuthServerInterface.cpp
CRUDP_Packet.cpp
CRUDP_Protocol.cpp
GameServerInterface.cpp
MapServerInterface.cpp
NetManager.cpp
ServerSupport.cpp
PacketCodec.cpp
GameProtocol.cpp
GameProtocolHandler.cpp
RoamingServer.cpp
ServerEndpoint.cpp
ServerHandle.cpp
ServerManager.cpp
InterfaceManager.cpp
Database.cpp
../include/AdminServerInterface.h
../include/AuthServerInterface.h
../include/Client.h
../include/CommonNetStructures.h
../include/CRUDP_Packet.h
../include/CRUDP_Protocol.h
../include/GameProtocol.h
../include/GameServerInterface.h
../include/MapServerInterface.h
../include/NetManager.h
../include/PacketCodec.h
../include/ServerHandle.h
../include/server_support.h
../include/Server.h
../include/RoamingServer.h
../include/InterfaceManager.h
../include/ServerManager.h
../include/Database.h
)
ADD_LIBRARY(segs_server_common STATIC ${segs_server_common_SRCS})
