CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

PROJECT(SEGS)
IF(MSVC)
    add_definitions( -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -DNOMINMAX -D_USE_MATH_DEFINES)
ELSE()
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11"  )
endif()
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(-D__STDC_FORMAT_MACROS)

SET(MAIN_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/3rd_party/include CACHE PATH "Includes")
SET(MAIN_LIB_PATH ${PROJECT_SOURCE_DIR}/3rd_party/lib CACHE PATH "Libraries")
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/out) # ERICEDIT: Changed the output directory to the build directory rather than source.
SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/out)
SET(CMAKE_INCLUDE_PATH ${MAIN_INCLUDE_PATH})
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts;${CMAKE_MODULE_PATH})
SET(UNICODE TRUE)
set(INSTALLED_TARGETS)
OPTION(ENABLE_SCRIPTING_ENGINE "Build experimental scripting engine ?" OFF)
OPTION(ENABLE_TESTS "Enable testing unit and functional tests" OFF)
OPTION(BUILD_COX_MAP_VIEWER "Build CoX map file viewer ?" OFF)

########################################################################################
# Prerequisites
########################################################################################
include(3rdparty_support)
set(ThirdParty_Install_Dir ${CMAKE_BINARY_DIR}/3rd_party/built) # ERICEDIT: Changed to CMAKE_BINARY_DIR so that this variable always points to the top level of the build directory.
add_subdirectory(3rd_party)
add_definitions(-DACE_HAS_DLL=1) # TODO: this should be set by per-target add_definitions
link_directories(${PROJECT_SOURCE_DIR}/lib) # ERICEDIT: Changed to lowercase.

find_package(Threads REQUIRED)
SET(QT_MISSING True)
# msvc only; mingw will need different logic
IF(MSVC)
    # look for user-registry pointing to qtcreator
    GET_FILENAME_COMPONENT(QT_BIN [HKEY_CURRENT_USER\\Software\\Classes\\Applications\\QtProject.QtCreator.cpp\\shell\\Open\\Command] PATH)

    # get root path so we can search for 5.3, 5.4, 5.5, etc
    STRING(REPLACE "/Tools" ";" QT_BIN "${QT_BIN}")
    LIST(GET QT_BIN 0 QT_BIN)
    FILE(GLOB QT_VERSIONS "${QT_BIN}/5.*")
    LIST(SORT QT_VERSIONS)

    # assume the latest version will be last alphabetically
    LIST(REVERSE QT_VERSIONS)

    LIST(GET QT_VERSIONS 0 QT_VERSION)

    # fix any double slashes which seem to be common
    STRING(REPLACE "//" "/"  QT_VERSION "${QT_VERSION}")

    # do some math trickery to guess folder
    # - qt uses (e.g.) "msvc2012"
    # - cmake uses (e.g.) "1800"
    # - see also https://cmake.org/cmake/help/v3.0/variable/MSVC_VERSION.html
    MATH(EXPR QT_MSVC "2000 + (${MSVC_VERSION} - 600) / 100")

    # check for 64-bit os
    # may need to be removed for older compilers as it wasn't always offered
    IF(CMAKE_SYSTEM_PROCESSOR MATCHES 64)
        SET(QT_MSVC "${QT_MSVC}_64")
    ENDIF()
    SET(QT_PATH "${QT_VERSION}/msvc${QT_MSVC}")
    SET(QT_MISSING False)
ENDIF()

# use Qt_DIR approach so you can find Qt after cmake has been invoked
IF(NOT QT_MISSING)
    MESSAGE("-- Qt found: ${QT_PATH}")
    SET(Qt5_DIR "${QT_PATH}/lib/cmake/Qt5/")
    SET(Qt5Test_DIR "${QT_PATH}/lib/cmake/Qt5Test")
ENDIF()
find_package(Qt5 COMPONENTS Core Sql REQUIRED)

link_directories(${ThirdParty_Install_Dir}/lib)
include_directories(${ThirdParty_Install_Dir}/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(Components)

add_subdirectory(Components)
add_subdirectory(Utilities)
add_subdirectory(Projects)
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/data)

