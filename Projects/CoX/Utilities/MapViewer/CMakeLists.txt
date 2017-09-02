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
#    src/QOSGWidget.cpp
#)

set(SOURCES
    src/MapViewerApp.cpp
    src/MapViewerApp.h
    src/main.cpp
)
ADD_EXECUTABLE(mapviewer ${SOURCES})
TARGET_LINK_LIBRARIES(mapviewer
    lutefisk3d_IMP
    ACE
)
qt5_use_modules(mapviewer Core)
