add_subdirectory(3rd_party)

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

#INCLUDE_DIRECTORIES(${target_INCLUDE_DIR} ${ACE_path})

#SET (target_SOURCES
#${gui_SOURCES}
#${target_CPP}
#${target_INCLUDE}
#${gui_MOC_SRC} ${gui_UI_H}
#)

#ADD_EXECUTABLE(mapviewer ${target_SOURCES})
ADD_EXECUTABLE(mapviewer src/main.cpp)
#ACE_ADD_LIBRARIES(mapviewer ACE)
TARGET_LINK_LIBRARIES(mapviewer
    Lutefisk3D
)
qt5_use_modules(mapviewer Core)
#ADD_DEPENDENCIES(mapviewer ${target_DEPENDS})
