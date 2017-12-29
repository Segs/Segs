CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

PROJECT(SEGS)
IF(MSVC)
    add_definitions( -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -DNOMINMAX -D_USE_MATH_DEFINES)
ELSE()
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11"  )
endif()
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-D__STDC_FORMAT_MACROS)

SET(MAIN_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/3rd_party/include CACHE PATH "Includes")
SET(MAIN_LIB_PATH ${PROJECT_SOURCE_DIR}/3rd_party/lib CACHE PATH "Libraries")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out)

SET(CMAKE_INCLUDE_PATH ${MAIN_INCLUDE_PATH})
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts;${CMAKE_MODULE_PATH})
SET(UNICODE TRUE)

OPTION(ENABLE_SCRIPTING_ENGINE "Build experimental scripting engine ?" OFF)
OPTION(ENABLE_TESTS "Enable testing unit and functional tests" OFF)
OPTION(BUILD_COX_MAP_VIEWER "Build CoX map file viewer ?" OFF)

########################################################################################
# Prerequisites
########################################################################################
include(3rdparty_support)
set(ThirdParty_Install_Dir ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/built)
add_subdirectory(3rd_party)
add_definitions(-DACE_HAS_DLL=1) # TODO: this should be set by per-target add_definitions
link_directories(${PROJECT_SOURCE_DIR}/lib) # ERICEDIT: Changed to lowercase.

find_package(Threads REQUIRED)
find_package(Qt5 COMPONENTS Core Sql REQUIRED)

link_directories(${ThirdParty_Install_Dir}/lib)
include_directories(${ThirdParty_Install_Dir}/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(Components)

add_subdirectory(Components)
add_subdirectory(Utilities)
add_subdirectory(Projects)
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/data)

########################################################################################
# DLL Transfers
########################################################################################
if(WIN32) # ERICEDIT: If the OS is Windows:
#    if(MSVC) # ERICEDIT: If this is the Visual Studio version of Qt:
#        # TODO: Copy the DLL files required for Visual Studio versions of the client.
#    else() # ERICEDIT: Else, this is Qt creator. Copy these DLLs.
#        get_target_property(QtCore_location_Release Qt5::Core LOCATION_Release) # ERICEDIT: Get the direct location of Qt5Core.dll.
#        get_filename_component(Qt_binary_location ${QtCore_location_Release} DIRECTORY) # ERICEDIT: Remove the file name and let the directory remain.
#        file(COPY ${Qt_binary_location}/libgcc_s_dw2-1.dll DESTINATION ${EXECUTABLE_OUTPUT_PATH})
#        file(COPY ${Qt_binary_location}/libstdc++-6.dll DESTINATION ${EXECUTABLE_OUTPUT_PATH})
#        file(COPY ${Qt_binary_location}/libwinpthread-1.dll DESTINATION ${EXECUTABLE_OUTPUT_PATH})
#        file(COPY ${QtCore_location_Release} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
#        file(COPY ${Qt_binary_location}/Qt5Gui.dll DESTINATION ${EXECUTABLE_OUTPUT_PATH})
#        file(COPY ${Qt_binary_location}/Qt5Network.dll DESTINATION ${EXECUTABLE_OUTPUT_PATH})
#        file(COPY ${Qt_binary_location}/Qt5Sql.dll DESTINATION ${EXECUTABLE_OUTPUT_PATH})
#        file(COPY ${Qt_binary_location}/Qt5Widgets.dll DESTINATION ${EXECUTABLE_OUTPUT_PATH})
#    endif()
    get_target_property(QtCore_location_Release Qt5::Core LOCATION_Release) # ERICEDIT: Get the direct location of Qt5Core.dll.
    get_filename_component(Qt_binary_location ${QtCore_location_Release} DIRECTORY) # ERICEDIT: Remove the file name and let the directory remain.
    add_custom_target(
        execTargets ALL
        DEPENDS ${EXECUTABLE_OUTPUT_PATH}/authserver.exe
        ${EXECUTABLE_OUTPUT_PATH}/binConverter.exe
        ${EXECUTABLE_OUTPUT_PATH}/dbtool.exe
        ${EXECUTABLE_OUTPUT_PATH}/ManifestBuilder.exe
        ${EXECUTABLE_OUTPUT_PATH}/piggtool.exe
        ${EXECUTABLE_OUTPUT_PATH}/slav.exe
        )
    find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt HINTS ${Qt_binary_location} ENV Qt_binary_location)
    add_custom_command(
        TARGET execTargets
        POST_BUILD
        COMMAND ${WINDEPLOYQT_EXECUTABLE} --release ${EXECUTABLE_OUTPUT_PATH}/authserver.exe
        COMMAND ${WINDEPLOYQT_EXECUTABLE} --release ${EXECUTABLE_OUTPUT_PATH}/binConverter.exe
        COMMAND ${WINDEPLOYQT_EXECUTABLE} --release ${EXECUTABLE_OUTPUT_PATH}/dbtool.exe
        COMMAND ${WINDEPLOYQT_EXECUTABLE} --release ${EXECUTABLE_OUTPUT_PATH}/ManifestBuilder.exe
        COMMAND ${WINDEPLOYQT_EXECUTABLE} --release ${EXECUTABLE_OUTPUT_PATH}/piggtool.exe
        COMMAND ${WINDEPLOYQT_EXECUTABLE} --release ${EXECUTABLE_OUTPUT_PATH}/slav.exe
        )
#    file(TO_NATIVE_PATH "${QWT_DIR}/lib/*${CMAKE_SHARED_LIBRARY_SUFFIX}" copySource)
#    # perform copy
#    ADD_CUSTOM_COMMAND(TARGET execTargets
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy ${copySource} ${EXECUTABLE_OUTPUT_PATH}
#    )
endif()

