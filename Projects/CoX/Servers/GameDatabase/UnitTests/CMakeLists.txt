# moc when necessary
set(CMAKE_AUTOMOC ON)

# moc files are generated in the binary dir, look for includes there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(GameDBEventRegistry GameDBEventRegistry.cpp)
add_test(GameDBEventRegistry GameDBEventRegistry)

target_link_libraries(GameDBEventRegistry
    Qt5::Test
    GameDatabase_interface
    SEGS_Components
)

set_target_properties (GameDBEventRegistry PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities/unit_tests
)
