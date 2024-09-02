cmake_minimum_required(VERSION 3.4)
PROJECT(SegsMonitor)

IF(CMAKE_CXX_COMPILER MATCHES cl.exe)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DWIN32 -DNOMINMAX)
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
ENDIF()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(ServerMonitor
    main.cpp
    MainWindow.h
    MainWindow.cpp
    MainWindow.ui
)

TARGET_LINK_LIBRARIES(ServerMonitor
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)
