# moc when necessary
set(CMAKE_AUTOMOC ON)

# moc files are generated in the binary dir, look for includes there
set(CMAKE_INCLUDE_CURRENT_DIR ON)
INCLUDE_DIRECTORIES(SYSTEM ${ACE_path})
add_executable(MapEventRegistry MapEventRegistry.cpp)
add_test(MapEventRegistry MapEventRegistry)

target_link_libraries(MapEventRegistry
    cox_server_common
    # FIXME: adding the above lib to linked libs fixes a very frustrating mingw ws2_32 linking problem
    # I have no idea why, and don't feel like digging into library/defines lists that are used in both cases
    Qt5::Test
    network_structures
    mapserver_lib
    gameData
    game_db_sync_lib
)
set_property(TARGET MapEventRegistry PROPERTY POSITION_INDEPENDENT_CODE ON)
