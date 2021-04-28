#FIND_PACKAGE(ACE REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

set(CMAKE_AUTOUIC TRUE)
set(CMAKE_AUTOMOC TRUE)
set(SOURCES
    src/CharViewerApp.cpp
    src/CharViewerApp.h
    src/PiggTools.cpp
    src/PiggTools.h
    src/CoHModelLoader.cpp
    src/CoHModelLoader.h
    src/CohModelConverter.cpp
    src/CohModelConverter.h
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
ADD_EXECUTABLE(charviewer ${SOURCES})
TARGET_LINK_LIBRARIES(charviewer
    Lutefisk3D
    ace_IMP
    gameData common_runtime cereal_IMP
    Qt5::Core Qt5::Widgets
)
# the following flag is need to compile on mingw
target_compile_options(charviewer PUBLIC -march=native)
#add_custom_command(TARGET mapviewer POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mapviewer_data
#    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ThirdParty_Install_Dir}/share/Lutefisk3D/CoreData ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mapviewer_data/CoreData
#    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Data ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mapviewer_data/Data
#)
# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES charviewer SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (charviewer PROPERTIES
    FOLDER Utilities
)
