cmake_minimum_required(VERSION 3.16)
project(SEGS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell cmake to honor CXX_VISIBILITY_PRESET and ignore it
cmake_policy(SET CMP0063 NEW)
cmake_policy(SET CMP0065 NEW)
cmake_policy(SET CMP0072 NEW)
cmake_policy(SET CMP0079 NEW)

# for version and revision info. Derived from https://github.com/dolphin-emu/dolphin/blob/master/CMakeLists.txt#L132
find_package(Git)
if(GIT_FOUND)
  # defines SEGS_REVISION
  execute_process(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
      OUTPUT_VARIABLE SEGS_REVISION
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  # defines SEGS_DESCRIPTION
  execute_process(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} describe --always --long --dirty
      OUTPUT_VARIABLE SEGS_DESCRIPTION
      OUTPUT_STRIP_TRAILING_WHITESPACE)

  # remove hash (and trailing "-0" if needed) from description
  string(REGEX REPLACE "(-0)?-[^-]+((-dirty)?)$" "\\2" SEGS_DESCRIPTION "${SEGS_DESCRIPTION}")

  # defines SEGS_BRANCH
  execute_process(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
      OUTPUT_VARIABLE SEGS_BRANCH
      OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

# version number
set(SEGS_VERSION_MAJOR "0")
set(SEGS_VERSION_MINOR "7")
set(SEGS_VERSION_PATCH "99")

# If SEGS is not built from a Git repository, default the version info to
# reasonable values.
if(NOT SEGS_REVISION)
    set(SEGS_DESCRIPTION "${SEGS_VERSION_MAJOR}.${SEGS_VERSION_MINOR}.${SEGS_VERSION_PATCH}")
    set(SEGS_REVISION "${SEGS_DESCRIPTION} (no further info)")
    set(SEGS_BRANCH "develop")
endif()

add_definitions(
    "-DSEGS_VERSION=\"${SEGS_VERSION_MAJOR}.${SEGS_VERSION_MINOR}.${SEGS_VERSION_PATCH}\""
    "-DSEGS_BRANCH=\"${SEGS_BRANCH}\""
    "-DSEGS_DESCRIPTION=\"${SEGS_DESCRIPTION}\""
    )

if(MSVC)
    add_definitions( -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -DNOMINMAX -D_USE_MATH_DEFINES)
    add_compile_options (/wd4251) # disable warning related to dll interface needed ... clients of class
    if (NOT (MSVC_VERSION LESS 1910))
      #add_compile_options (/MP)
      add_compile_options (/permissive-)
      #add_compile_options (/d2cgsummary)
      #add_compile_options (/d1reportTime)
      add_compile_options (/diagnostics:caret)
      #add_compile_options (/sdl)
      #add_compile_options (/arch:AVX2)
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wuseless-cast")
    if(MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wuseless-cast -Wno-nested-anon-types -Wno-gnu-anonymous-struct -Wno-gnu-zero-variadic-macro-arguments")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pedantic -Wall -Wextra -Wuseless-cast -Wno-nested-anon-types -Wno-gnu-anonymous-struct -Wno-gnu-zero-variadic-macro-arguments")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(AUTOGEN_TARGETS_FOLDER ON)

add_definitions(-D__STDC_FORMAT_MACROS)

set(MAIN_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/3rd_party/include CACHE PATH "Includes")
set(MAIN_LIB_PATH ${PROJECT_SOURCE_DIR}/3rd_party/lib CACHE PATH "Libraries")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/out)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/out)
set(CMAKE_INCLUDE_PATH ${MAIN_INCLUDE_PATH})
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts;${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/3rd_party/prebuilt")
set(UNICODE TRUE)
set(INSTALLED_TARGETS)
option(ENABLE_SCRIPTING_ENGINE "Build experimental scripting engine ?" ON)
option(ENABLE_TESTS "Enable testing unit and functional tests" OFF)
option(BUILD_COX_MAP_VIEWER "Build CoX map file viewer ?" OFF)
option(BUILD_RPC_TEST_CLIENT "Build RPC Test Client ?" OFF)
option(BUILD_CLIENT_PATCH "Build client patch for AMD graphics cards ?" OFF)

########################################################################################
# Prerequisites
########################################################################################
find_package(Threads REQUIRED)
include(LocateQt5)
find_package(Qt5 5.12 COMPONENTS Core Sql REQUIRED)

include(SDKHelper)
include(3rdparty_support)
set(ThirdParty_Install_Dir ${CMAKE_BINARY_DIR}/3rd_party/built) # ERICEDIT: Changed to CMAKE_BINARY_DIR so that this variable always points to the top level of the build directory.
add_subdirectory(3rd_party)
link_directories(${PROJECT_SOURCE_DIR}/lib) # ERICEDIT: Changed to lowercase.

# Temporarily commented out as failing to build
# if(MSVC)
#     find_package(SegsEngine REQUIRED)
# endif()

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
        set_target_properties (${testname} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities/unit_tests
        )
    endmacro()
endif()

link_directories(${ThirdParty_Install_Dir}/lib)
include_directories(SYSTEM ${ThirdParty_Install_Dir}/include)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/3rd_party)
include_directories(${PROJECT_SOURCE_DIR}/include)

# this has to be visited before scan_event_types macro below has been used
add_subdirectory(Utilities)

# this function is here to solve the env paths issue
function(to_path_list var path1)
    if("${CMAKE_HOST_SYSTEM}" MATCHES ".*Windows.*")
        set(sep "\\;")
    else()
        set(sep ":")
    endif()
    set(result "${path1}") # First element doesn't require separator at all...
    foreach(path ${ARGN})
        set(result "${result}${sep}${path}") # .. but other elements do.
    endforeach()
    set(${var} "${result}" PARENT_SCOPE)
endfunction()
##############################################
# Event registration support functionality
macro(scan_event_types group) # followed by a list of directories to scan
    set(all_headers)
    foreach(path ${ARGN})
        message("Scanning ${path} for events")
        file(GLOB headers ${path}/*.h)
        list(APPEND all_headers ${headers})
    endforeach()
    to_path_list(path_with_qt5 $ENV{PATH} ${Qt5_BIN_DIR} )
    # for event_gen we first add qt5 binary path to allow it to run during build.
    add_custom_command(
        COMMAND ${CMAKE_COMMAND} -E env "PATH=${path_with_qt5}" $<SHELL_PATH:$<TARGET_FILE:event_gen>> scan_events ${group} ${CMAKE_CURRENT_BINARY_DIR}/ ${ARGN}
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/${group}.gen.cpp
        DEPENDS event_gen ${all_headers}
        COMMENT "Generating ${group} implementation source"
    )
endmacro()
macro(add_event_sources group tgt_var)
    list(APPEND ${tgt_var} ${CMAKE_CURRENT_BINARY_DIR}/${group}.gen.cpp)
    message("adding ${group} to sources")
endmacro()
macro(add_messages_library name )
    #find handler's events
    scan_event_types(${name}Events ${CMAKE_CURRENT_SOURCE_DIR})
    add_event_sources(${name}Events ${name}_SOURCES)
    add_library(${name}_interface STATIC ${${name}_SOURCES} ${ARGN})
    # allow all headers to refer to events by their dir/name.h
    target_include_directories(${name}_interface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_link_libraries(${name}_interface PUBLIC Qt5::Core)
    # all interfaces use stuff from Common, so should depend/link to it
    target_link_libraries(${name}_interface PUBLIC SEGS_Components)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ARGN})
    set_target_properties (${name}_interface PROPERTIES
            FOLDER Common/Messages
            RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities/unit_tests
    )
endmacro()

macro(add_handler name)
    # TODO: consider add_subdirectory(../../Common/Messages/${name}) to force existence/building of interface lib
    # add the implementation library
    add_library(${name} STATIC ${ARGN})
    target_link_libraries(${name} PUBLIC ${name}_interface)
endmacro()
##############################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Components)
add_subdirectory(Common)
#add_subdirectory(Editor) Temporarily commented out as failing to build
add_subdirectory(Servers)
add_subdirectory(Clients)

# Add README, scripts, settings.cfg, and Starter Databases
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/README.md ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/README.md COPYONLY)

add_subdirectory(Data)

file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/default_setup)
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/logs)
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
