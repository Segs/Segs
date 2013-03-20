CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(SEGS)
IF(CMAKE_CXX_COMPILER MATCHES cl.exe)
    add_definitions( -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
ELSE()
   SET(CMAKE_CXX_FLAGS "-Wall -std=c++11"  )
endif()
add_definitions(-D__STDC_FORMAT_MACROS)

SET(MAIN_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/3rd_party/include CACHE PATH "Includes")
SET(MAIN_LIB_PATH ${PROJECT_SOURCE_DIR}/3rd_party/lib CACHE PATH "Libraries")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out)

SET(CMAKE_INCLUDE_PATH ${MAIN_INCLUDE_PATH})
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts;${CMAKE_MODULE_PATH})
SET(UNICODE TRUE)

OPTION(SCRIPTING_ENGINE "Build experimental scripting engine ?" OFF)

SET(PostgreSQL_ADDITIONAL_VERSIONS "9.2")
########################################################################################
# Prerequisites
########################################################################################

find_package(ACE REQUIRED) # ACE framework
find_package(Threads REQUIRED)
find_package(PostgreSQL REQUIRED) # PostgreSQL
if(SCRIPTING_ENGINE)
    find_package(MRuby REQUIRED) # MRuby
endif()

include(SEGS_Components)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common/include)

add_subdirectory(Components)
add_subdirectory(Projects)
