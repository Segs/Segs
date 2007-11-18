SET(UNICODE )
IF(WIN32)
  IF (CMAKE_CXX_COMPILER MATCHES "^cl$")
#	SET ( CMAKE_CXX_FLAGS_DEBUG "/MTd /Zi /Od /RTC1" )
  ENDIF (CMAKE_CXX_COMPILER MATCHES "^cl$")
ENDIF(WIN32)

# ACE framework
FIND_PATH(ACE_path ACE.h
  ${MAIN_INCLUDE_PATH}/ace
  /usr/local/include/ace
  /usr/include/ace
)
FIND_LIBRARY(ACE_lib 
                NAMES 
                    libACE.so ACE ACEd.lib libACE.a 
                PATHS 
                    /usr/lib 
                    /usr/local/lib 
                    ${MAIN_LIB_PATH}
                DOC
                    "Path to ACE framework library"]
)

FIND_PACKAGE(Boost)
FIND_PACKAGE(SWIG)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Common/include)
INCLUDE_DIRECTORIES(${ACE_path})
INCLUDE_DIRECTORIES(${BOOST_path})

ADD_SUBDIRECTORY(examples)
