CMAKE_MINIMUM_REQUIRED(VERSION 3.4)
PROJECT(SEGS)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

IF(MSVC)
    add_definitions( -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -DNOMINMAX -D_USE_MATH_DEFINES)
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wuseless-cast -std=c++14"  )
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wuseless-cast -std=c++14 -Wno-nested-anon-types -Wno-gnu-anonymous-struct -Wno-gnu-zero-variadic-macro-arguments")
endif()
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
find_package(Threads REQUIRED)

include(3rdparty_support)
set(ThirdParty_Install_Dir ${CMAKE_BINARY_DIR}/3rd_party/built) # ERICEDIT: Changed to CMAKE_BINARY_DIR so that this variable always points to the top level of the build directory.
add_subdirectory(3rd_party)
add_definitions(-DACE_HAS_DLL=1) # TODO: this should be set by per-target add_definitions
link_directories(${PROJECT_SOURCE_DIR}/lib) # ERICEDIT: Changed to lowercase.

SET(QT_MISSING True)
# msvc only; mingw will need different logic
IF(MSVC)
    # look for user-registry pointing to qtcreator
    GET_FILENAME_COMPONENT(QT_BIN [HKEY_CURRENT_USER\\Software\\Classes\\Applications\\QtProject.QtCreator.cpp\\shell\\Open\\Command] PATH)
    if(${MSVC_VERSION} VERSION_LESS "1910")
        MESSAGE(FATAL_ERROR "SEGS requires visual studio 2017 to build")
    endif()

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

    # we only support 2017, and qt5 uses vs2015 files for 32 bit build
    SET(QT_MSVC "2015")
    # check for 64-bit os
    # may need to be removed for older compilers as it wasn't always offered
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
        SET(QT_MSVC "2017_64")
    endif()

    SET(QT_PATH "${QT_VERSION}/msvc${QT_MSVC}")
    SET(QT_MISSING False)
    file(TO_NATIVE_PATH "${QT_PATH}/bin" QT_BIN_DIR_WINDOWS)
    set(CMAKE_MSVCIDE_RUN_PATH "${QT_BIN_DIR_WINDOWS}" CACHE STATIC "MSVC IDE Run path" FORCE)
ENDIF()

# use Qt_DIR approach so you can find Qt after cmake has been invoked
IF(NOT QT_MISSING)
    MESSAGE("-- Qt found: ${QT_PATH}")
    SET(Qt5_DIR "${QT_PATH}/lib/cmake/Qt5/")
    SET(Qt5Test_DIR "${QT_PATH}/lib/cmake/Qt5Test")
ENDIF()
find_package(Qt5 COMPONENTS Core Sql REQUIRED)
if(ENABLE_TESTS)
    enable_testing()
    find_package(Qt5 COMPONENTS Test REQUIRED)
    # Event registration support functionality
    macro(add_segs_test testname) # followed by a list of directories to scan
        add_executable(${testname} ${testname}.cpp)
        add_test(${testname} ${testname})

        target_link_libraries(${testname}
            Qt5::Test
            ${ARGN}
        )
    endmacro()
endif()
##############################################
# Event registration support functionality
macro(scan_event_types group) # followed by a list of directories to scan
    set(all_headers)
    foreach(path ${ARGN})
        message("Scanning ${path} for events")
        file(GLOB headers ${path}/*.h)
        list(APPEND all_headers ${headers})
    endforeach()

    add_custom_command(
        COMMAND event_gen scan_events ${group} ${CMAKE_CURRENT_BINARY_DIR}/ ${ARGN}
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/${group}.gen.cpp
        DEPENDS event_gen ${all_headers}
        COMMENT "Generating ${group} implementation source"
    )
endmacro()
macro(add_event_sources group tgt_var)
    list(APPEND ${tgt_var} ${CMAKE_CURRENT_BINARY_DIR}/${group}.gen.cpp)
    message("adding ${group} to sources")
endmacro()
##############################################

link_directories(${ThirdParty_Install_Dir}/lib)
include_directories(SYSTEM ${ThirdParty_Install_Dir}/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(Components)

add_subdirectory(Components)
add_subdirectory(Utilities)
add_subdirectory(Projects)

file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/data)

set(MAPNAMES
  City_00_01
  City_01_01 City_01_02 City_01_03
  City_02_01 City_02_02
  City_03_01 City_03_02
  City_04_01 City_04_02
  City_05_01
  Hazard_01_01
  Hazard_02_01
  Hazard_03_01
  Hazard_04_01 Hazard_04_02
  Hazard_05_01
  Trial_01_01 Trial_01_02
  Trial_02_01
  Trial_03_01
  Trial_04_01 Trial_04_02
  Trial_05_01
  )
foreach(MAPDIR ${MAPNAMES})
  file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/DefaultMapInstances/${MAPDIR})
endforeach(MAPDIR)
