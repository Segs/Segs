if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Lutefisk3D/CMake/LutefiskOptions.cmake")
	message(FATAL_ERROR "Lutefisk3D git submodule has not been checked out.")
endif()
if(FALSE)
    set(LUTEFISK3D_NETWORK OFF) # don't need it, and it doesn't compile anyway :)
    add_subdirectory(Lutefisk3D)
else()
	#NOTE: by default lutefisk builds in RelWithDebInfo
	# for our use case this seems the most optimal build type.
    libname(lutefisk3d Lutefisk3D)
    # add the main options for lutefisk, 
    include(${CMAKE_CURRENT_SOURCE_DIR}/Lutefisk3D/CMake/LutefiskOptions.cmake)
    set(LUTEFISK3D_NETWORK OFF) # don't need it, and it doesn't compile anyway :)

    set(lutefisk_cmake_options
    -DQt5_DIR:PATH=${Qt5_DIR}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_STAGING_PREFIX:PATH=${ThirdParty_Install_Dir}
    -DCMAKE_INSTALL_PREFIX:PATH=${ThirdParty_Install_Dir}
    )
    if(MINGW)
        set(IMPLIB_PATH ${ThirdParty_Install_Dir}/lib/libLutefisk3D_RelWithDebugInfo.dll.a )
    endif()
    # forward LUTEFISK3D_* options to the external project build.
    get_cmake_property(__cmake_variables VARIABLES)
    foreach (var ${__cmake_variables})
        if ("${var}" MATCHES "^LUTEFISK3D_")
            set(lutefisk_cmake_options ${lutefisk_cmake_options} -D${var}:BOOL=${${var}})
        endif ()
    endforeach()

    ExternalProject_Add(
       l3d_BUILD
       SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Lutefisk3D
       UPDATE_COMMAND ""
       INSTALL_DIR ${ThirdParty_Install_Dir}
       CMAKE_ARGS ${lutefisk_cmake_options}
       CMAKE_GENERATOR "${CMAKE_GENERATOR}"
       CMAKE_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET}
       BUILD_BYPRODUCTS ${lutefisk3d_LIBRARY_SHARED} ${lutefisk3d_LIBRARY_IMP} ${IMPLIB_PATH}
       EXCLUDE_FROM_ALL ON # don't build this when build-all is requested ?
    )
    add_library(Lutefisk3D SHARED IMPORTED GLOBAL)
    target_include_directories(Lutefisk3D INTERFACE ${ThirdParty_Install_Dir}/include)
    add_dependencies(Lutefisk3D l3d_BUILD)
    #set_shared_lib_properties(lutefisk3d)
    set_property(TARGET Lutefisk3D APPEND PROPERTY IMPORTED_LOCATION ${lutefisk3d_LIBRARY_SHARED} )
    set_property(TARGET Lutefisk3D APPEND PROPERTY IMPORTED_LOCATION_DEBUG ${lutefisk3d_LIBRARY_SHARED} )
    set_property(TARGET Lutefisk3D APPEND PROPERTY IMPORTED_LOCATION_RELEASE ${lutefisk3d_LIBRARY_SHARED} )

    # add lutefisk defines
    foreach (var ${__cmake_variables})
        if ("${var}" MATCHES "^LUTEFISK3D_")
            if ("${${var}}")
                target_compile_definitions(Lutefisk3D INTERFACE -D${var})
            endif ()
        endif ()
    endforeach()

    if(MINGW)
        set_property(TARGET Lutefisk3D PROPERTY IMPORTED_IMPLIB ${IMPLIB_PATH} )
        set_property(TARGET Lutefisk3D PROPERTY IMPORTED_IMPLIB_DEBUG ${IMPLIB_PATH} )
        set_property(TARGET Lutefisk3D PROPERTY IMPORTED_IMPLIB_RELWITHDEBINFO ${IMPLIB_PATH} )
    else()
        set_property(TARGET Lutefisk3D APPEND PROPERTY IMPORTED_IMPLIB ${lutefisk3d_LIBRARY_IMP} )
        set_property(TARGET Lutefisk3D APPEND PROPERTY IMPORTED_IMPLIB_RELEASE ${lutefisk3d_LIBRARY_IMP} )
        if(MSVC)
            set_property(TARGET Lutefisk3D APPEND PROPERTY IMPORTED_IMPLIB_DEBUG ${lutefisk3d_LIBRARY_IMP} )
        else()
            set_property(TARGET Lutefisk3D APPEND PROPERTY IMPORTED_IMPLIB_DEBUG ${lutefisk3d_LIBRARY_IMP} )
        endif()
    endif()
    #set_property(TARGET Lutefisk3D APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS ACE_HAS_DLL=1)
endif()
