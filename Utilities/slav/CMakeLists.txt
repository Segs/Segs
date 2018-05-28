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
