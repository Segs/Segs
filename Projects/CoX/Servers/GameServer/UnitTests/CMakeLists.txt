# moc when necessary
set(CMAKE_AUTOMOC ON)

# moc files are generated in the binary dir, look for includes there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(GameEventRegistry GameEventRegistry.cpp)
add_test(GameEventRegistry GameEventRegistry)

target_link_libraries(GameEventRegistry
    Qt5::Test
    GameServer
    common_runtime
)
