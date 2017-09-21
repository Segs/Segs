#FIND_PACKAGE(ACE REQUIRED)
find_package(Qt5Core REQUIRED)

################# Qt Ui related ###############################################
#include(${QT_USE_FILE})
#SET(gui_UI_FILES
#    ui/mainwindow.ui
#    ui/objecttreewidget.ui
#)
#SET(gui_MOC_HEADERS
#    include/mainwindow.h
#    include/objecttreewidget.h
#    include/mapmodel.h
#    include/CompositeViewerQOSG.h
#    include/QOSGWidget.h
#)
#SET(gui_SOURCES
#    src/mainwindow.cpp
#    src/objecttreewidget.cpp
#    src/mapmodel.cpp
#    src/CompositeViewerQOSG.cpp
#)

set(CMAKE_AUTOUIC TRUE)
set(CMAKE_AUTOMOC TRUE)
set(SOURCES
    src/MapViewerApp.cpp
    src/MapViewerApp.h
    src/PiggTools.cpp
    src/PiggTools.h
    src/CoHSceneConverter.h
    src/CohTextureConverter.cpp
    src/CohTextureConverter.h
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
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ThirdParty_Install_Dir}/share/Lutefisk3D/CoreData ${EXECUTABLE_OUTPUT_PATH}/CoreData
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Data ${EXECUTABLE_OUTPUT_PATH}/Data
)
