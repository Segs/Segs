add_library(sol2 INTERFACE)
target_include_directories(sol2 INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lua_sol2/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS sol2 EXPORT sol2 DESTINATION lib) # ignored
install(EXPORT sol2 FILE sol2-config.cmake DESTINATION share/cmake/sol2)
install(FILES lua_sol2/include/sol/sol.hpp DESTINATION include/sol)
install(FILES lua_sol2/include/sol/config.hpp DESTINATION include/sol)

