find_package(Qt5 5.8.0 COMPONENTS Core Gui Widgets OpenGL Network REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(SLAV_SOURCES
    SLAVLogic
    ServerConnection
    UpdaterDlg.cpp
    UpdaterDlg.h
    UpdaterDlg.ui
    ProjectDescriptor
    ProjectManifest
    FileSignatureWorker
    main.cpp
)

add_executable(slav ${SLAV_SOURCES})
target_link_libraries(slav cereal_IMP  Qt5::Core Qt5::Widgets Qt5::Network)

add_executable(ManifestBuilder ManifestBuilder.cpp ProjectManifest.cpp FileSignatureWorker)
target_link_libraries(ManifestBuilder cereal_IMP Qt5::Core)
# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES slav SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
get_target_property(TARGET_SOURCES2 ManifestBuilder SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES2}) 
set_target_properties (slav PROPERTIES
    FOLDER Utilities
    RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities
)
set_target_properties (ManifestBuilder PROPERTIES
    FOLDER Utilities
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities
)
