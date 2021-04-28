set(SegsEngine_RELEASE 4.0.0.14)
set(build_from_source ON)
if(MSVC)
    set(build_from_source OFF)
endif()

if(build_from_source)
    libname(editor_engine editor_engine)
    ExternalProject_Add(
        editor_engine-${SegsEngine_RELEASE}
        URL https://github.com/Segs/SegsEngine/archive/v${SegsEngine_RELEASE}-alpha.tar.gz
        #SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SE2
        INSTALL_DIR ${PROJECT_SOURCE_DIR}/3rd_party/prebuilt
        CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_STAGING_PREFIX:PATH=<INSTALL_DIR> -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        CMAKE_GENERATOR "${CMAKE_GENERATOR}"
        CMAKE_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET}
        BUILD_BYPRODUCTS ${SegsEngine_LIBRARY_STATIC}
    )

    add_library(SegsEngine::EASTL_Import INTERFACE IMPORTED)
    file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/include/SegsEngine)
    file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/bin)
    file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/lib)

    set_target_properties(SegsEngine::EASTL_Import PROPERTIES
      INTERFACE_COMPILE_DEFINITIONS "EASTL_USER_CONFIG_HEADER=\"EASTL/SegsEngine_config.h\""
      INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/include"

    )

    add_executable(SegsEngine::binding_generator IMPORTED GLOBAL)
    add_dependencies(SegsEngine::binding_generator editor_engine-${SegsEngine_RELEASE})

    add_library(SegsEngine::editor_engine SHARED IMPORTED GLOBAL)
    add_dependencies(SegsEngine::editor_engine editor_engine-${SegsEngine_RELEASE})

    # Create imported target SegsEngine::editor_interface
    add_library(SegsEngine::editor_interface INTERFACE IMPORTED)

    set_target_properties(SegsEngine::editor_interface PROPERTIES
      INTERFACE_COMPILE_DEFINITIONS "TOOLS_ENABLED;DEBUG_ENABLED"
      INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/include/SegsEngine;${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/include/"
      INTERFACE_LINK_LIBRARIES "Qt5::Core;SegsEngine::EASTL_Import"
    )
    set_property(TARGET SegsEngine::editor_engine APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    if(WIN32)
        set_target_properties(SegsEngine::binding_generator PROPERTIES
            IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/bin/binding_generator.exe"
        )
    set_target_properties(SegsEngine::editor_engine PROPERTIES
          IMPORTED_LOCATION_DEBUG "${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/bin/editor_engine.dll"
      IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/lib/editor_engine.lib"
      IMPORTED_SONAME_DEBUG "libeditor_engine.so"
      INTERFACE_LINK_LIBRARIES "SegsEngine::editor_interface"
    )
    else()
        set_target_properties(SegsEngine::binding_generator PROPERTIES
            IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/bin/binding_generator"
        )
        set_target_properties(SegsEngine::editor_engine PROPERTIES
          IMPORTED_LOCATION_DEBUG "${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/bin/libeditor_engine.so"
          IMPORTED_SONAME_DEBUG "libeditor_engine.so"
          INTERFACE_LINK_LIBRARIES "SegsEngine::editor_interface"
        )
    endif()

else()

check_and_update_binary_deps(SegsEngine SegsEngine ${PROJECT_SOURCE_DIR}/3rd_party/prebuilt ${SegsEngine_RELEASE})

endif()

###########################################################################################
macro(set_engine_plugin_options )
    set(oneValueArgs NAME CLASSPROP TYPE SHARED DISABLED)
    set(multiValueArgs INCLUDES DEFINES SOURCES LIBS DEP_TARGETS)
    cmake_parse_arguments(plugin_options "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    set(name ${plugin_options_NAME})
    set(classprop ${plugin_options_CLASSPROP})
    set(includes ${plugin_options_INCLUDES})
    set(sources ${plugin_options_SOURCES})
    set(defines ${plugin_options_DEFINES})
    set(libs ${plugin_options_LIBS})
    set(targets_dep ${plugin_options_DEP_TARGETS})
    set(when_shared_libs ${plugin_options_WHEN_SHARED_LIBS})
    set(group_folder ${plugin_options_TYPE})
    set(tgt_name segs_plugin_${name})
    message("Adding a plugin ${tgt_name}")
    add_library(${tgt_name} SHARED)
    set_target_properties(${tgt_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/plugins/segs)
    set_target_properties(${tgt_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/out/plugins/segs)
    set_target_properties(${tgt_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/out/plugins/segs)
    set_target_properties(${tgt_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/plugins/segs)
    target_link_libraries(${tgt_name} PRIVATE SegsEngine::editor_engine ${libs})
    set_target_properties(${tgt_name} PROPERTIES AUTOMOC TRUE)
    target_sources(${tgt_name} PRIVATE ${sources})
    if(includes)
        target_include_directories(${tgt_name} PRIVATE ${includes})
    endif()
    if(defines)
        target_compile_definitions(${tgt_name} PUBLIC ${defines})
    endif()
    set_target_properties (${tgt_name} PROPERTIES
        FOLDER plugins/${group_folder}
    )
    foreach(tgt ${targets_dep})
        add_dependencies(${tgt} ${tgt_name})
    endforeach()
endmacro()

