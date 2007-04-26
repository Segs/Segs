# $Id: CMakeLists.txt 253 2006-08-31 22:00:14Z malign $

SET (adminserver_SRCS 
AdminServer.cpp
AdminDatabase.cpp
../include/AdminDatabase.h
../include/AdminServer.h

)
ADD_LIBRARY(adminserver_lib STATIC ${adminserver_SRCS})
TARGET_LINK_LIBRARIES(adminserver_lib segs_common)
ADD_DEPENDENCIES(adminserver_lib segs_common)
