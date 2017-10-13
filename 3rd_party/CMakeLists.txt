link_directories(${ThirdParty_Install_Dir}/lib)

include(ExternalProject_CEREAL.cmake)
include(ExternalProject_ACE.cmake)
if(BUILD_COX_MAP_VIEWER)
include(ExternalProject_Lutefisk3D.cmake)
endif()
########################################################################################
# Optional components
########################################################################################

if(ENABLE_SCRIPTING_ENGINE)
    include(ExternalProject_LUA.cmake)
    include(ExternalProject_SOL2.cmake)
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
