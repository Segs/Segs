find_package(Qt5 5.9 COMPONENTS Core Gui Widgets OpenGL Network REQUIRED)

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
    resources.qrc

)

add_executable(SEGSAdmin WIN32 ${SEGSADMIN_SOURCES})
qt5_use_modules(SEGSAdmin Core Gui Widgets Network)
