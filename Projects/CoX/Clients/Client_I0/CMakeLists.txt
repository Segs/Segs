LINK_DIRECTORIES(../patch_tools)
set (CMAKE_CXX_STANDARD 17)
if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()
set(SOURCES
    utils/dll_patcher.cpp
    patch_all_the_things.cpp
    dll_core.cpp
)
add_library(cohdll SHARED ${SOURCES})
# we link to the imp lib for the exe
target_link_libraries(cohdll CityOfPatches)
# enforce the dll prefix.
SET_TARGET_PROPERTIES(cohdll PROPERTIES PREFIX "lib")