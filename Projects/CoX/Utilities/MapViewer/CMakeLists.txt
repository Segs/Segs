#FIND_PACKAGE(ACE REQUIRED)
find_package(Qt5Core REQUIRED)

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
    lutefisk3d_IMP
    ACE
    gameData cereal_IMP
)
qt5_use_modules(mapviewer Core Widgets  )

add_custom_command(TARGET mapviewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ThirdParty_Install_Dir}/share/Lutefisk3D/CoreData ${EXECUTABLE_OUTPUT_PATH}/mapviewer
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Data ${EXECUTABLE_OUTPUT_PATH}/data
    COMMAND ${CMAKE_COMMAND} -E make_directory ${EXECUTABLE_OUTPUT_PATH}/data/bin
    COMMAND ${CMAKE_COMMAND} -E make_directory ${EXECUTABLE_OUTPUT_PATH}/scripts
)
