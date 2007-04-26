# $Id: CMakeLists.txt 306 2007-01-23 12:16:25Z nemerle $
SET (mapserver_SRCS
SEGSMap.cpp
MapClient.cpp
MapHandler.cpp
MapServer.cpp
MapServerEndpoint.cpp
MapPacket.cpp
Entity.cpp
EntityStorage.cpp
../../Common/src/RoamingServer.cpp
../include/SEGSMap.h
../include/EntityStorage.h
../include/MapClient.h
../include/MapPacket.h
../include/MapServer.h
../include/MapServerEndpoint.h
../include/MapHandler.h
)
ADD_LIBRARY(mapserver_lib STATIC ${mapserver_SRCS})
#TARGET_LINK_LIBRARIES(gameserver_lib segs_common)
ADD_DEPENDENCIES(mapserver_lib segs_common)
