find_package(Qt5 5.8 COMPONENTS Core Gui Widgets OpenGL Network REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(SEGSADMIN_SOURCES
    SEGSAdminTool.ui
    SEGSAdminTool.cpp
    SEGSAdminTool.h
    main.cpp
    AddNewUserDialog.ui
    AddNewUserDialog.cpp
    AddNewUserDialog.h
    GenerateConfigFileDialog.cpp
    GenerateConfigFileDialog.h
    GenerateConfigFileDialog.ui
    resources.qrc
    SetUpData.ui
    SetUpData.cpp
    SetUpData.h
    Globals.cpp
    Globals.h
    GetIPDialog.cpp
    GetIPDialog.h
    GetIPDialog.ui
    SettingsDialog.cpp
    SettingsDialog.h
    SettingsDialog.ui
    NetworkManager.cpp
    NetworkManager.h
    UpdateDetailDialog.cpp
    UpdateDetailDialog.h
    UpdateDetailDialog.ui
    AboutDialog.cpp
    AboutDialog.h
    AboutDialog.ui
    TextEdit.cpp
    TextEdit.h
    SelectScriptDialog.cpp
    SelectScriptDialog.h
    SelectScriptDialog.ui
    SyntaxHighlighter.cpp
    SyntaxHighlighter.h
    Worker.cpp
    Worker.h
    Helpers.cpp
    Helpers.h
)
add_executable(SEGSAdmin WIN32 ${SEGSADMIN_SOURCES})
target_link_libraries(SEGSAdmin PUBLIC SEGS_Components Qt5::Core Qt5::Widgets Qt5::Network)

IF(WIN32)
    include(Windeployqt)
    windeployqt(SEGSAdmin deploy)
ELSEIF(APPLE)
    include(Macdeployqt)
    macdeployqt(SEGSAdmin deploy)
ENDIF()

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES SEGSAdmin SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (SEGSAdmin PROPERTIES
    FOLDER Utilities
)
