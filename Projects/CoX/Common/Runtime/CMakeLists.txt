find_package(Qt5 COMPONENTS Core REQUIRED)
set(COMMON_RUNTIME_SRC
    SceneGraph.h
    SceneGraph.cpp
)
add_library(common_runtime STATIC ${COMMON_RUNTIME_SRC})
add_dependencies(common_runtime gameData)
#add_dependencies(common_runtimea ace_IMP cereal_IMP) # used for Singleton stuff.
target_link_libraries(common_runtime PUBLIC Qt5::Core)
target_include_directories(common_runtime INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  #$<INSTALL_INTERFACE:common_runtime>  # <prefix>/include/mylib
)
target_link_libraries(common_runtime INTERFACE gameData )
#set_property(TARGET segs_runtime APPEND PROPERTY INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:cereal_IMP,INTERFACE_INCLUDE_DIRECTORIES>")
