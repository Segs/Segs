find_package(Qt5 5.8 COMPONENTS Core Gui Widgets OpenGL Network REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(RPCTESTCLIENT_SOURCES
    RPCTestClient.cpp
    RPCTestClient.h
    RPCTestClient.ui
    main.cpp
)
add_executable(RPCTestClient ${RPCTESTCLIENT_SOURCES})
target_link_libraries(RPCTestClient PUBLIC Qt5::Core Qt5::Widgets Qt5::Network jcon_IMP ace_IMP)

IF(WIN32)
    include(Windeployqt)
    windeployqt(RPCTestClient deploy)
ENDIF()
if(APPLE)
    include(Macdeployqt)
    macdeployqt(RPCTestClient deploy)
endif()

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES RPCTestClient SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (RPCTestClient PROPERTIES
    FOLDER Utilities
)
