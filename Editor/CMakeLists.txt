
###########################################################################################
add_subdirectory(plugins)

set(SOURCES
    AuthClient.h
    AuthClient.cpp
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

