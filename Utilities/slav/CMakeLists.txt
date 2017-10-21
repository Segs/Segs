
find_package(Qt5Core 5.8.0 REQUIRED)
find_package(Qt5Gui 5.8.0 REQUIRED)
find_package(Qt5Widgets 5.8.0 REQUIRED)
find_package(Qt5OpenGL 5.8.0 REQUIRED)
find_package(Qt5Network 5.8.0 REQUIRED)
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
qt5_use_modules(slav Core Gui Widgets Network)
target_link_libraries(slav cereal_IMP)

add_executable(ManifestBuilder ManifestBuilder.cpp ProjectManifest.cpp FileSignatureWorker)
qt5_use_modules(ManifestBuilder Core)
target_link_libraries(ManifestBuilder cereal_IMP)
