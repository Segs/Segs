CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

PROJECT(CoH_Patch)
IF(MSVC)
    add_definitions( -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -DNOMINMAX -D_USE_MATH_DEFINES)
ELSE()
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11"  )
endif()
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

LINK_DIRECTORIES(../patch_tools)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set (CMAKE_CXX_STANDARD 17)
if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()
set(DLL_SOURCES
    renderer/RendererUtils.cpp
    renderer/RendererUtils.h
    graphics/gfx.cpp
    graphics/gfx.h
)
set(SOURCES
    ${DLL_SOURCES}
    utils/dll_patcher.cpp
    patch_all_the_things.cpp
    dll_core.cpp
)
add_library(cohdll SHARED ${SOURCES})
# we link to the imp lib for the exe
target_link_libraries(cohdll CityOfPatches)
# enforce the dll prefix.
SET_TARGET_PROPERTIES(cohdll PROPERTIES PREFIX "lib"
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)

