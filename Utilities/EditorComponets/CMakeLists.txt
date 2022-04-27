cmake_minimum_required(VERSION 3.17)
project(SegsEd)

cmake_policy(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


set(sources
    main.cpp
    SegsOriginEditor.cpp
    SegsOriginEditor.h
    SegsOriginEditor.ui
    soe.qrc
)

add_executable(SegsEd ${sources})

target_link_libraries(SegsEd PUBLIC Qt5::Widgets)
set_target_properties(SegsEd PROPERTIES AUTOMOC TRUE)
set_target_properties(SegsEd PROPERTIES AUTOUIC TRUE)
set_target_properties(SegsEd PROPERTIES AUTORCC TRUE)
