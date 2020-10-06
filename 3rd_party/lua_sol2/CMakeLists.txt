cmake_minimum_required (VERSION 3.1)
project (lua_sol2)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(lua_sol2 INTERFACE)
target_include_directories(lua_sol2 INTERFACE
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
$<INSTALL_INTERFACE:include>
)
install(TARGETS lua_sol2 EXPORT lua_sol2 DESTINATION lib) # ignored
install(EXPORT lua_sol2 FILE lua_sol2-config.cmake DESTINATION share/cmake/lua_sol2)
install(FILES include/sol/sol.hpp DESTINATION include/sol)
install(FILES include/sol/config.hpp DESTINATION include/sol)
install(FILES include/sol/sol_forward.hpp DESTINATION include/sol)
