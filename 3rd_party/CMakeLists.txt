link_directories(${ThirdParty_Install_Dir}/lib)

include(ExternalProject_CEREAL.cmake)
include(ExternalProject_ACE.cmake)
if(MINGW) # ERICEDIT: If the OS is Windows:
    add_custom_target( # ERICEDIT: Add a target that depends on authserver.
        libace ALL
        DEPENDS ${EXECUTABLE_OUTPUT_PATH}/authserver.exe
        )
    add_custom_command( # ERICEDIT: Copy over libACE.dll to the output directory.
        TARGET libace
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ace_LIBRARY_SHARED} ${EXECUTABLE_OUTPUT_PATH}
        )
endif()
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
include(ExternalProject_JCON.cmake)

