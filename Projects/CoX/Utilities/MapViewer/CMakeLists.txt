# $Id: CMakeLists.txt 216 2009-12-06 19:37:58Z nemerle $

FIND_PACKAGE(ACE REQUIRED)
FIND_PACKAGE(OpenSceneGraph REQUIRED osgUtil)
FIND_PACKAGE(Qt COMPONENTS Gui Core REQUIRED)

################# Qt Ui related ###############################################
include(${QT_USE_FILE})
SET(gui_UI_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/mainwindow.ui
)
SET(gui_MOC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mainwindow.h
)
SET(gui_SOURCES
    src/mainwindow.cpp
    ${gui_MOC_HEADERS}
)
QT4_WRAP_UI(gui_UI_H ${gui_UI_FILES})
QT4_WRAP_CPP(gui_MOC_SRC ${gui_MOC_HEADERS})
qt4_automoc(${gui_SOURCES})
###############################################################################

INCLUDE_DIRECTORIES(${OPENSCENEGRAPH_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}) # to access moc compiled headers

SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/LoadMap.cpp
)
SET(target_INCLUDE
)
SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(target_DEPENDS "")

SEGS_ADD_COMPONENT(static_storage LIB)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${gui_SOURCES}
${target_CPP}
${target_INCLUDE}
${gui_MOC_SRC} ${gui_UI_H}
)




ADD_EXECUTABLE(mapviewer ${target_SOURCES})

TARGET_LINK_LIBRARIES(mapviewer   ${target_DEPENDS}
    ${ACE_lib}
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSCENEGRAPH_LIBRARIES}
    ${QT_LIBRARIES}
)
ADD_DEPENDENCIES(mapviewer ${target_DEPENDS})

