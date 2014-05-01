CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(SEGS)
IF(CMAKE_CXX_COMPILER MATCHES cl.exe)
    add_definitions( -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
ELSE()
   SET(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++11"  )
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
include(ExternalProject)
ExternalProject_Add(
   ACE_BUILD
   URL http://nemerle.eu/naked_ace.tar.gz
   SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd_party/naked_ace
   UPDATE_COMMAND ""
   INSTALL_DIR ${PROJECT_SOURCE_DIR}/3rd_party/
   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)
SET(ACE_path ${PROJECT_SOURCE_DIR}/3rd_party)
SET(ACE_LIBRARIES ACE)

ExternalProject_add(
   SQLite_BUILD
   SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd_party/naked_sqlite
   DOWNLOAD_COMMAND ""
   UPDATE_COMMAND ""
   INSTALL_DIR ${PROJECT_SOURCE_DIR}/3rd_party/
   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)
SET(SQLite_path ${PROJECT_SOURCE_DIR}/3rd_party)
SET(SQLite_LIBRARIES sqlite)

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
link_directories(${PostgreSQL_LIBRARY_DIRS})
#-DBUILD_OSG_APPLICATIONS:BOOL=OFF -DOSG_GL1_AVAILABLE:BOOL=OFF -DOSG_GL2_AVAILABLE:BOOL=OFF -DOSG_GL_DISPLAYLISTS_AVAILABLE:BOOL=OFF -DOSG_GL_FIXED_FUNCTION_AVAILABLE:BOOL=OFF -DOSG_GL_MATRICES_AVAILABLE:BOOL=OFF -DOSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE:BOOL=OFF -DOSG_GL_VERTEX_FUNCS_AVAILABLE:BOOL=OFF -DOSG_USE_QT:BOOL=OFF

########################################################################################
# Optional components
########################################################################################

if(ENABLE_SCRIPTING_ENGINE)
    ADD_DEFINITIONS(-DSCRIPTING_ENABLED=1)
    FIND_PACKAGE(Ruby REQUIRED)
    ExternalProject_Add(
        MRUBY_BUILD
        DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/3rd_party/mruby
        GIT_REPOSITORY git://github.com/mruby/mruby.git
        GIT_TAG master # will have to set it to a specific git-hash in the future.
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd_party/mruby
        BUILD_COMMAND ${RUBY_EXECUTABLE} ./minirake
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND ""
        INSTALL_DIR ${PROJECT_SOURCE_DIR}/3rd_party/
    )
    SET(MRuby_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/3rd_party/mruby/include)
    LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/3rd_party/mruby/build/host/lib)
    SET(MRuby_LIBRARIES mruby)
endif()

if(ENABLE_TESTS)
    enable_testing()
    ExternalProject_Add(
        GMOCK_BUILD
        DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/3rd_party/gmock
        URL http://googlemock.googlecode.com/files/gmock-1.6.0.zip
        UPDATE_COMMAND ""
        PATCH_COMMAND patch -N -t -p1 < ../gmock.patch
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd_party/gmock
        INSTALL_DIR ${PROJECT_SOURCE_DIR}/3rd_party/
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    )
    set(GTest_LIBRARIES gtest gtest_main ${CMAKE_THREAD_LIBS_INIT})
endif()
link_directories(${PROJECT_SOURCE_DIR}/3rd_party/lib)
include_directories(${PROJECT_SOURCE_DIR}/3rd_party/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

include(SEGS_Components)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common/include)

add_subdirectory(Components)
add_subdirectory(Projects)
