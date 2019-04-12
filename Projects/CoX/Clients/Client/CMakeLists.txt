set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Quick REQUIRED)

add_executable(qml_dummy "main.cpp" "qml.qrc")

target_link_libraries(qml_dummy Qt5::Core Qt5::Quick)


install(TARGETS qml_dummy
    DESTINATION deploy
)
IF(WIN32)
    include(Windeployqt)
    windeployqt(qml_dummy deploy)
ELSEIF(APPLE)
    include(Macdeployqt)
    macdeployqt(qml_dummy deploy)
ENDIF()

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES qml_dummy SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES})
set_target_properties (qml_dummy PROPERTIES
    FOLDER Projects/CoX/Clients
    RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities
)
