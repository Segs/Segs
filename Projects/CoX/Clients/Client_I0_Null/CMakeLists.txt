CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

PROJECT(CoH_Null_Patch)
IF(MSVC)
    add_definitions( -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -DNOMINMAX -D_USE_MATH_DEFINES)
    if (MSVC_VERSION GREATER_EQUAL "1900")
        include(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
        if (_cpp_latest_flag_supported)
            add_compile_options("/std:c++latest")
        endif()
    endif()
ELSE()
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11"  )
endif()
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../../../CMakeScripts;${CMAKE_MODULE_PATH})
MESSAGE(${CMAKE_SOURCE_DIR}/../../../../CMakeScripts)
include(3rdparty_support)
set(ThirdParty_Install_Dir ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/built)

#add_subdirectory(3rd_party)

link_directories(
    ${ThirdParty_Install_Dir}/lib
    ../patch_tools
)
include_directories(
    ${ThirdParty_Install_Dir}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    include
)
set(SOURCES
    ${DLL_SOURCES}
    utils/dll_patcher.cpp
    patch_all_the_things.cpp
    dll_core.cpp
)
add_library(cohdll SHARED ${SOURCES})
# enforce the dll prefix.
SET_TARGET_PROPERTIES(cohdll PROPERTIES PREFIX "lib"
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)

