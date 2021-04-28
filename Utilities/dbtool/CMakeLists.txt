SET(CMAKE_CXX_STANDARD 14)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -DNOMINMAX)
    ADD_DEFINITIONS(/W4)
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")
ENDIF()
find_package(Qt5 COMPONENTS Core Sql REQUIRED)

add_subdirectory(migrations)

set(tool_SRC
    main.cpp
    DatabaseConfig.cpp
    DatabaseConfig.h
    DBConnection.cpp
    DBConnection.h
    DBConnection_AddUser.cpp
    DBConnection_Create.cpp
    DBConnection_Helpers.cpp
    DBConnection_Upgrade.cpp
    DBMigrationStep.cpp
    DBMigrationStep.h
    # Migrations are added programatically
    ${MIGRATION_SOURCES}
)

add_executable(dbtool ${tool_SRC})
target_include_directories(dbtool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dbtool Qt5::Core Qt5::Sql)
target_link_libraries(dbtool SEGS_Components)

# dbtool relies on QT more now, so we need these for windows builds
IF(WIN32)
    include(Windeployqt)
    windeployqt(dbtool deploy)
ELSEIF(APPLE)
    include(Macdeployqt)
    macdeployqt(dbtool deploy)
ENDIF()

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES dbtool SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (dbtool PROPERTIES
    FOLDER Utilities
    RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities
)

# hack to prevent errors with FOLDER properties when using generated files
target_sources(dbtool PRIVATE ${MIGRATION_GEN})
