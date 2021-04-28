find_package(Qt5 COMPONENTS Core REQUIRED)
set(COMMON_RUNTIME_SRC
    Animation.cpp
    Animation.h
    AnimationEngine.cpp
    AnimationEngine.h
    Handle.h
    Model.cpp
    Model.h
    Prefab.cpp
    Prefab.h
    RuntimeData.cpp
    RuntimeData.h
    SceneGraph.h
    SceneGraph.cpp
    Sequencer.cpp
    Sequencer.h
    Texture.cpp
    Texture.h
    Systems/FXSystem.cpp
    Systems/FXSystem.h
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

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES common_runtime SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (common_runtime PROPERTIES
    FOLDER Common
)
