set(SegsEngine_RELEASE 4.0.0.6-alpha)
if(NOT MSVC)
libname(editor_engine editor_engine)
ExternalProject_Add(
    editor_engine-${SegsEngine_RELEASE}
    URL https://github.com/Segs/SegsEngine/archive/v${SegsEngine_RELEASE}.tar.gz
    INSTALL_DIR ${PROJECT_SOURCE_DIR}/3rd_party/prebuilt
    CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_STAGING_PREFIX:PATH=${ThirdParty_Install_Dir} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET}
    BUILD_BYPRODUCTS ${SegsEngine_LIBRARY_STATIC}
)

add_library(SegsEngine::EASTL_Import INTERFACE IMPORTED)

set_target_properties(SegsEngine::EASTL_Import PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "EASTL_USER_CONFIG_HEADER=\"EASTL/SegsEngine_config.h\""
  INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/include"
  
)


add_library(SegsEngine::editor_engine SHARED IMPORTED GLOBAL)
add_dependencies(SegsEngine::editor_engine editor_engine-${SegsEngine_RELEASE})

# Create imported target SegsEngine::editor_interface
add_library(SegsEngine::editor_interface INTERFACE IMPORTED)

set_target_properties(SegsEngine::editor_interface PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "TOOLS_ENABLED"
  INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/include/SegsEngine;${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/include/"
  INTERFACE_LINK_LIBRARIES "Qt5::Core;SegsEngine::EASTL_Import"
)
set_property(TARGET SegsEngine::editor_engine APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(SegsEngine::editor_engine PROPERTIES
  IMPORTED_LOCATION_DEBUG "${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/bin/libeditor_engine.so"
  IMPORTED_SONAME_DEBUG "libeditor_engine.so"
  INTERFACE_LINK_LIBRARIES "SegsEngine::editor_interface"
)
  
else()

check_and_update_binary_deps(SegsEngine SegsEngine ${PROJECT_SOURCE_DIR}/3rd_party/prebuilt ${SegsEngine_RELEASE})

endif()
