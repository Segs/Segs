# $Id: CMakeLists.txt 270 2006-10-01 18:33:49Z nemerle $

PROJECT(SEGS)

SET(UNICODE )

IF(WIN32)
  IF (CMAKE_CXX_COMPILER MATCHES "^cl$")
#	SET ( CMAKE_CXX_FLAGS_DEBUG "/MTd /Zi /Od /RTC1" )
  ENDIF (CMAKE_CXX_COMPILER MATCHES "^cl$")
ENDIF(WIN32)

# just testing 
OPTION(BUILD_SEPARATE "When set to 1 this will build each server as a separate executable file" ON)

# ACE framework
FIND_PATH(ACE_path ACE.h
  /usr/local/include/ace
  /usr/include/ace
)
FIND_LIBRARY(ACE_lib NAMES libACE.so ACE ACEd.lib libACE.a PATHS /usr/lib /usr/local/lib DOC "Path to ACE framework library"])

# PostgreSQL
FIND_PATH(BOOST_path object_pool.hpp
   /usr/local/include/boost
   /usr/include/boost/
   /include/boost
)
# PostgreSQL
FIND_PATH(PGSQL_path libpq-fe.h
   /usr/local/include/postgresql
   /usr/include/postgresql
   /usr/include/pgsql
)
FIND_LIBRARY(PGSQL_lib NAMES libpq.so libpq.a libpq pq PATHS /usr/lib /usr/local/lib DOC "Path to PostgreSQL library"])

# zlib 

FIND_PACKAGE(ZLIB REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(SWIG)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Common/include)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ACE_path})
INCLUDE_DIRECTORIES(${PGSQL_path})
INCLUDE_DIRECTORIES(${BOOST_path})


ADD_SUBDIRECTORY(Common)
ADD_SUBDIRECTORY(AdminServer)
ADD_SUBDIRECTORY(GameServer)
ADD_SUBDIRECTORY(MapServer)
ADD_SUBDIRECTORY(AuthServer)

EXPORT_LIBRARY_DEPENDENCIES(DEPENDENT.TXT)
