#FIND_PACKAGE(ACE REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

set(CMAKE_AUTOUIC TRUE)
set(CMAKE_AUTOMOC TRUE)
set(SOURCES
    src/MapViewerApp.cpp
    src/MapViewerApp.h
    src/PiggTools.cpp
    src/PiggTools.h
    src/CoHModelLoader.cpp
    src/CoHModelLoader.h
    src/CohModelConverter.cpp
    src/CohModelConverter.h
    src/CoHSceneConverter.cpp
    src/CoHSceneConverter.h
    src/CohTextureConverter.cpp
    src/CohTextureConverter.h
    src/SideWindow.cpp
    src/SideWindow.h
    src/SideWindow.ui
    src/DataPathsDialog.cpp
    src/DataPathsDialog.h
    src/DataPathsDialog.ui
    src/main.cpp
)
ADD_EXECUTABLE(mapviewer ${SOURCES})
TARGET_LINK_LIBRARIES(mapviewer
    Lutefisk3D
    ace_IMP
    gameData common_runtime cereal_IMP
    Qt5::Core Qt5::Widgets
)
# the following flag is need to compile on mingw
target_compile_options(mapviewer PUBLIC -march=native)
#add_custom_command(TARGET mapviewer POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities/mapviewer_data
#    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ThirdParty_Install_Dir}/share/Lutefisk3D/CoreData ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities/mapviewer_data/CoreData
#    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Data ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities/mapviewer_data/Data
#)
# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES mapviewer SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES})
set_target_properties (mapviewer PROPERTIES
    FOLDER Utilities
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities
)
