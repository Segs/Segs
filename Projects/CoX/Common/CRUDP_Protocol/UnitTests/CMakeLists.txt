# moc when necessary
set(CMAKE_AUTOMOC ON)

# moc files are generated in the binary dir, look for includes there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(CRUDPEventRegistry CRUDPEventRegistry.cpp)
add_test(CRUDPEventRegistry CRUDPEventRegistry)

target_link_libraries(CRUDPEventRegistry
    Qt5::Test
    AuthProtocol
    SEGS_Components
    CRUDP_Protocol
)
