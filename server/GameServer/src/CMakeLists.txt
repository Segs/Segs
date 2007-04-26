# $Id: CMakeLists.txt 294 2006-10-12 17:52:28Z nemerle $

SET (gameserver_SRCS 
Character.cpp
CharacterClient.cpp
CharacterDatabase.cpp
ClientHandler.cpp
GamePacket.cpp
GameServer.cpp
GameServerEndpoint.cpp
../include/Character.h
../include/CharacterClient.h
../include/CharacterDatabase.h
../include/ClientHandler.h
../include/GameServer.h
../include/GamePacket.h
../include/GameServerEndpoint.h
)
ADD_LIBRARY(gameserver_lib STATIC ${gameserver_SRCS})
#TARGET_LINK_LIBRARIES(gameserver_lib segs_common)
ADD_DEPENDENCIES(gameserver_lib segs_common)
