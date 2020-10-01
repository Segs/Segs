

return()

if(MSVC)
    find_package(SegsEngine REQUIRED)
else()
    include(${PROJECT_SOURCE_DIR}/3rd_party/ExternalProject_SegsEngine.cmake)
endif()

###########################################################################################
macro(set_plugin_options )
    set(oneValueArgs NAME CLASSPROP TYPE SHARED DISABLED)
    set(multiValueArgs INCLUDES DEFINES SOURCES LIBS)
    cmake_parse_arguments(plugin_options "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    set(name ${plugin_options_NAME})
    set(classprop ${plugin_options_CLASSPROP})
    set(includes ${plugin_options_INCLUDES})
    set(sources ${plugin_options_SOURCES})
    set(defines ${plugin_options_DEFINES})
    set(libs ${plugin_options_LIBS})
    set(when_shared_libs ${plugin_options_WHEN_SHARED_LIBS})
    set(group_folder ${plugin_options_TYPE})
    set(tgt_name segs_plugin_${name})
    message("Adding a plugin ${tgt_name}")
    add_library(${tgt_name} SHARED)
    set_target_properties(${tgt_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/plugins/segs)
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

endmacro()

###########################################################################################
add_subdirectory(plugins)

set(SOURCES
    main.cpp
    os_specific/app.rc
)
add_executable(SegsEditor ${SOURCES})
target_link_libraries(SegsEditor SegsEngine::editor_engine)

# copy engine dlls and plugins to editor executable directory
add_custom_command(
        TARGET SegsEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/3rd_party/prebuilt/bin
                ${CMAKE_BINARY_DIR}/out)

