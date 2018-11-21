# moc when necessary
set(CMAKE_AUTOMOC ON)

# moc files are generated in the binary dir, look for includes there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(AuthEventRegistry AuthEventRegistry.cpp)
add_test(AuthEventRegistry AuthEventRegistry)

target_link_libraries(AuthEventRegistry
    Qt5::Test
    AuthProtocol
)

set_target_properties (AuthEventRegistry PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities/unit_tests
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities/unit_tests
)
