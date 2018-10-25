# moc when necessary
set(CMAKE_AUTOMOC ON)

# moc files are generated in the binary dir, look for includes there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(AuthDBEventRegistry AuthDBEventRegistry.cpp)
add_test(AuthDBEventRegistry AuthDBEventRegistry)

target_link_libraries(AuthDBEventRegistry
    Qt5::Test
    AuthDatabase
)
