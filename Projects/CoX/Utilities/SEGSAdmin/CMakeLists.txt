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

)
add_executable(SEGSAdmin WIN32 ${SEGSADMIN_SOURCES})
target_link_libraries(SEGSAdmin PUBLIC jcon_IMP Qt5::Core Qt5::Widgets Qt5::Network)
