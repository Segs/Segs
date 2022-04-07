link_directories(${ThirdParty_Install_Dir}/lib)

include(ExternalProject_CEREAL.cmake)
include(ExternalProject_ACE.cmake)
if(MINGW OR MSVC) # ERICEDIT: If the OS is Windows:
    add_custom_target( # ERICEDIT: Add a target that depends on authserver.
        libace ALL
        DEPENDS ace_IMP
        )
    add_custom_command( # ERICEDIT: Copy over libACE.dll to the output directory.
        TARGET libace
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ace_LIBRARY_SHARED} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
endif()

########################################################################################
# Optional components
########################################################################################
include(ExternalProject_LUA.cmake)
include(ExternalProject_SOL2.cmake)
include(ExternalProject_JCON.cmake)
#include(ExternalProject_SegsEngine.cmake) Temporarily commented out as failing to build
