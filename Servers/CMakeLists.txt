set(UNICODE )

# just testing
option(BUILD_SEPARATE "When set to 1 this will build each server as a separate executable file" ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(SYSTEM ${ACE_path})

add_subdirectory(GameDatabase)
add_subdirectory(AuthDatabase)
add_subdirectory(GameServer)
add_subdirectory(MapServer)
add_subdirectory(AuthServer)
