Project(ace)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out/lib/${CMAKE_BUILD_TYPE})
IF(WIN32)
    SET(OS_NAME win32)
ENDIF()
IF(UNIX)
    SET(OS_NAME linux)
ENDIF()
IF(APPLE) # warning this is not foolproof
    SET(OS_NAME macosx)
ENDIF()
# warning this writes into projects root directory
CONFIGURE_FILE(ace/config.h.cmake ${PROJECT_SOURCE_DIR}/ace/config.h)

ADD_SUBDIRECTORY( ace )
#        @KOKYU@ \
#        @ASNMP@ \
#        @ACEXML@ \
#        apps \
#        bin \
#        netsvcs \
#        @protocols@ \
#        websvcs

if(BUILD_EXAMPLES)
ADD_SUBDIRECTORY( examples )
endif()

if(BUILD_TESTS)
ADD_SUBDIRECTORY( performance-tests tests )
endif()
