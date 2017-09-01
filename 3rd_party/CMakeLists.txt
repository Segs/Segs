link_directories(${ThirdParty_Install_Dir}/lib)

include(ExternalProject_CEREAL.cmake)
include(ExternalProject_ACE.cmake)

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
