include(ExternalProject)

include(ExternalProject_CEREAL.cmake)


ExternalProject_Add(
   ACE_BUILD
   URL http://nemerle.eu/naked_ace.tar.gz
   SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd_party/naked_ace
   UPDATE_COMMAND ""
   INSTALL_DIR ${ThirdParty_Install_Dir}
   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)
SET(ACE_path ${ThirdParty_Install_Dir} PARENT_SCOPE)
SET(ACE_LIBRARIES ACE PARENT_SCOPE)

ExternalProject_Add(
   OSG_BUILD
   URL http://nemerle.eu/naked_osg.tar.gz
   SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd_party/naked_osg
   UPDATE_COMMAND ""
   INSTALL_DIR ${ThirdParty_Install_Dir}
   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DBUILD_OSG_APPLICATIONS:BOOL=OFF -DOSG_GL1_AVAILABLE:BOOL=OFF -DOSG_GL2_AVAILABLE:BOOL=OFF -DOSG_GL_DISPLAYLISTS_AVAILABLE:BOOL=OFF -DOSG_GL_FIXED_FUNCTION_AVAILABLE:BOOL=OFF -DOSG_GL_MATRICES_AVAILABLE:BOOL=OFF -DOSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE:BOOL=OFF -DOSG_GL_VERTEX_FUNCS_AVAILABLE:BOOL=OFF -DOSG_USE_QT:BOOL=OFF -DLIB_POSTFIX=
)
SET(OSG_DIR ${ThirdParty_Install_Dir} PARENT_SCOPE)
SET(OPENSCENEGRAPH_LIBRARIES osgDB osg PARENT_SCOPE)

IF(CMAKE_CXX_COMPILER MATCHES cl.exe)
    SET(OPENSCENEGRAPH_LIBRARIES osgDBd osgd PARENT_SCOPE)
ELSE()
    SET(OPENSCENEGRAPH_LIBRARIES  osgDB osg PARENT_SCOPE)
endif()

ExternalProject_add(
   SQLite_BUILD
   SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd_party/naked_sqlite
   DOWNLOAD_COMMAND ""
   UPDATE_COMMAND ""
   INSTALL_DIR ${ThirdParty_Install_Dir}
   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)
SET(SQLite_path ${ThirdParty_Install_Dir} PARENT_SCOPE)
SET(SQLite_LIBRARIES sqlite PARENT_SCOPE)

ExternalProject_add(
   Zlib_BUILD
   SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd_party/zlib
   DOWNLOAD_COMMAND ""
   UPDATE_COMMAND ""
   INSTALL_DIR ${ThirdParty_Install_Dir}
   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)
SET(ZLIB_path ${ThirdParty_Install_Dir} PARENT_SCOPE)
SET(ZLIB_INCLUDE_DIR ${ZLIB_path}/include PARENT_SCOPE)
IF(CMAKE_CXX_COMPILER MATCHES cl.exe)
    SET(ZLIB_LIBRARIES zlibd PARENT_SCOPE)
ELSE()
    SET(ZLIB_LIBRARIES z PARENT_SCOPE)
endif()

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
