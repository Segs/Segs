find_package(Qt5 COMPONENTS Core REQUIRED)
add_executable(event_gen main.cpp)
target_link_libraries(event_gen Qt5::Core)

IF(WIN32)
    include(Windeployqt)
    windeployqt(event_gen deploy)
ENDIF()
