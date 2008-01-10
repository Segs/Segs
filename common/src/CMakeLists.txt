# $Id: CMakeLists.txt 301 2006-12-26 15:50:44Z nemerle $

SET (segs_common_SRCS 
BitStream.cpp
Buffer.cpp 
PacketBase.cpp
CRUDP_Packet.cpp
CRUDP_Protocol.cpp
PacketCodec.cpp
GameProtocol.cpp
GameProtocolHandler.cpp
../include/Base.h
../include/BitStream.h
../include/Buffer.h
../include/ConfigExtension.h
../include/PacketBase.h
../include/version.h
../include/CRUDP_Packet.h
../include/CRUDP_Protocol.h
../include/GameProtocol.h
../include/GameProtocolHandler.h
../include/PacketCodec.h
)

ADD_LIBRARY(segs_common STATIC ${segs_common_SRCS})
