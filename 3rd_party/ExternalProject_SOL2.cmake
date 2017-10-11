ExternalProject_Add(
   sol2_BUILD
   URL ""
   SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lua_sol2
   UPDATE_COMMAND ""
   INSTALL_DIR ${ThirdParty_Install_Dir}
   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
   CMAKE_GENERATOR "${CMAKE_GENERATOR}"
   CMAKE_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET}
)
add_library(sol2_IMP INTERFACE IMPORTED GLOBAL)
add_dependencies(sol2_IMP sol2_BUILD)
#TODO: cmake workaround
file(MAKE_DIRECTORY ${ThirdParty_Install_Dir}/include)
set_property(TARGET sol2_IMP PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${ThirdParty_Install_Dir}/include)
