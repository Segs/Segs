CMAKE_MINIMUM_REQUIRED(VERSION 2.8.10)

PROJECT(SEGS)
IF(CMAKE_CXX_COMPILER MATCHES cl.exe)
    add_definitions( -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -DNOMINMAX -DACE_BUILD_DLL)
ELSE()
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11"  )
endif()
add_definitions(-D__STDC_FORMAT_MACROS)

SET(MAIN_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/3rd_party/include CACHE PATH "Includes")
SET(MAIN_LIB_PATH ${PROJECT_SOURCE_DIR}/3rd_party/lib CACHE PATH "Libraries")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out)

SET(CMAKE_INCLUDE_PATH ${MAIN_INCLUDE_PATH})
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts;${CMAKE_MODULE_PATH})
SET(UNICODE TRUE)

OPTION(ENABLE_SCRIPTING_ENGINE "Build experimental scripting engine ?" OFF)
OPTION(ENABLE_TESTS "Enable testing unit and functional tests" OFF)

SET(PostgreSQL_ADDITIONAL_VERSIONS "9.2")
########################################################################################
# Prerequisites
########################################################################################
set(ThirdParty_Install_Dir ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/built)
add_subdirectory(3rd_party)

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib)

find_package(Threads REQUIRED)
find_package(Qt5Core REQUIRED)

if(PostgreSQL_FOUND)
link_directories(${PostgreSQL_LIBRARY_DIRS})
endif()

link_directories(${ThirdParty_Install_Dir}/lib)
include_directories(${ThirdParty_Install_Dir}/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

include(SEGS_Components)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common/include)

add_subdirectory(Components)
add_subdirectory(Projects)
