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

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES GameEventRegistry SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (GameEventRegistry PROPERTIES
    FOLDER UnitTests/GameServer/GameEventRegistry
    RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities/unit_tests
)
