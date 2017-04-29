include(ExternalProject)
function(encode_lib_name basename)
    if(UNIX)
        set(${basename}_LIBRARY_SHARED lib${basename}.so PARENT_SCOPE)
        set(${basename}_LIBRARY_SHARED_D lib${basename}.so PARENT_SCOPE)
        set(${basename}_LIBRARY_STATIC lib${basename}.a PARENT_SCOPE)
        set(${basename}_LIBRARY_STATIC_D lib${basename}.a PARENT_SCOPE)
        set(${basename}_LIBRARY_IMP ${basename}.so PARENT_SCOPE)
        set(${basename}_LIBRARY_IMP_D ${basename}.so PARENT_SCOPE)
    elseif(MSVC)
        set(${basename}_LIBRARY_SHARED ${basename}.dll PARENT_SCOPE)
        set(${basename}_LIBRARY_SHARED_D ${basename}d.dll PARENT_SCOPE)
        set(${basename}_LIBRARY_STATIC ${basename}.lib PARENT_SCOPE)
        set(${basename}_LIBRARY_STATIC_D ${basename}d.lib PARENT_SCOPE)
        set(${basename}_LIBRARY_IMP ${basename}.lib PARENT_SCOPE)
        set(${basename}_LIBRARY_IMP_D ${basename}d.lib PARENT_SCOPE)
    elseif(MINGW)
        set(${basename}_LIBRARY_STATIC lib${basename}.a PARENT_SCOPE)
        set(${basename}_LIBRARY_STATIC_D lib${basename}.a PARENT_SCOPE)
        set(${basename}_LIBRARY_SHARED lib${basename}.dll PARENT_SCOPE)
        set(${basename}_LIBRARY_SHARED_D lib${basename}.dll PARENT_SCOPE)
        set(${basename}_LIBRARY_IMP lib${basename}.dll PARENT_SCOPE)
        set(${basename}_LIBRARY_IMP_D lib${basename}.dll PARENT_SCOPE)
    endif()
endfunction()
function(libname tgt basename)
    encode_lib_name(${basename})
    set(${tgt}_LIBRARY_STATIC ${ThirdParty_Install_Dir}/lib/${${basename}_LIBRARY_STATIC} PARENT_SCOPE)
    set(${tgt}_LIBRARY_STATIC_D ${ThirdParty_Install_Dir}/lib/${${basename}_LIBRARY_STATIC_D} PARENT_SCOPE)
    set(${tgt}_LIBRARY_SHARED ${ThirdParty_Install_Dir}/lib/${${basename}_LIBRARY_SHARED} PARENT_SCOPE)
    set(${tgt}_LIBRARY_SHARED_D ${ThirdParty_Install_Dir}/lib/${${basename}_LIBRARY_SHARED_D} PARENT_SCOPE)
    set(${tgt}_LIBRARY_IMP ${ThirdParty_Install_Dir}/lib/${${basename}_LIBRARY_IMP} PARENT_SCOPE)
    set(${tgt}_LIBRARY_IMP_D ${ThirdParty_Install_Dir}/lib/${${basename}_LIBRARY_IMP_D} PARENT_SCOPE)
endfunction()
macro(set_shared_lib_properties basename)
    SET_PROPERTY(TARGET ${basename}_IMP APPEND PROPERTY IMPORTED_LOCATION ${ace_LIBRARY_SHARED} )
    SET_PROPERTY(TARGET ${basename}_IMP APPEND PROPERTY IMPORTED_LOCATION_DEBUG ${${basename}_LIBRARY_SHARED_D} )
    SET_PROPERTY(TARGET ${basename}_IMP APPEND PROPERTY IMPORTED_LOCATION_RELEASE ${${basename}_LIBRARY_SHARED} )
    SET_PROPERTY(TARGET ${basename}_IMP APPEND PROPERTY IMPORTED_IMPLIB ${${basename}_LIBRARY_IMP} )
    SET_PROPERTY(TARGET ${basename}_IMP APPEND PROPERTY IMPORTED_IMPLIB_RELEASE ${${basename}_LIBRARY_IMP} )
    SET_PROPERTY(TARGET ${basename}_IMP APPEND PROPERTY IMPORTED_IMPLIB_DEBUG ${${basename}_LIBRARY_IMP_D} )
endmacro()
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
