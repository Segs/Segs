find_package(Qt5 COMPONENTS Core REQUIRED)
add_executable(event_gen main.cpp)
target_link_libraries(event_gen Qt5::Core)

IF(WIN32)
    include(Windeployqt)
    windeployqt(event_gen deploy)
ENDIF()
# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES event_gen SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (event_gen PROPERTIES
    FOLDER Utilities
    RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities
)
