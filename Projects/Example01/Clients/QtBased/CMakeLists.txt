FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
FIND_PACKAGE(OpenGL)
include(${QT_USE_FILE})

SET(gui_HEADERS
	include/clientwindow.h
)
SET(gui_UI_FILES
	ui/clientwindow.ui
)

SET(gui_MOC_HEADERS ${gui_HEADERS}
)
SET(gui_SOURCES
    src/clientwindow.cpp
    ${gui_MOC_HEADERS}
)

QT4_WRAP_UI(gui_UI_H ${gui_UI_FILES})
QT4_WRAP_CPP(gui_MOC_SRC ${gui_MOC_HEADERS})
QT4_AUTOMOC(${gui_SOURCES})
#QT4_ADD_RESOURCES(gui_RES_SOURCES resources/filename.qrc)

SET(target_CPP
    src/main.cpp
    ${gui_RES_SOURCES}
    ${gui_SOURCES}
     ${gui_MOC_SRC}
)
SET(target_INCLUDE
#    include/filename.h
    ${gui_UI_H}
)

SET(target_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}

)
SET(target_DEPENDS "")

# N_ADD_COMPONENT(Config INCLUDES)

INCLUDE_DIRECTORIES(
    ${target_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
#    ${ACE_PATH}
)
# WIN32
ADD_EXECUTABLE(StickHeoroes_client ${target_CPP} ${gui_UI_H})
#ADD_DEPENDENCIES(StickHeoroes_client ${target_DEPENDS})
TARGET_LINK_LIBRARIES(StickHeoroes_client 
#    glew32
    ${target_DEPENDS}
    ${QT_LIBRARIES}
#    ${QT_QTMAIN_LIBRARY}
    ${OPENGL_LIBRARIES}
#    optimized
#	${QT_QTMAIN_LIBRARY}
#    optimized
#	OIS_static
#    debug
#	    OIS_static_d
)
