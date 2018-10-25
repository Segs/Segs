project(jcon_ex)

include_directories(${CMAKE_SOURCE_DIR}/src)

file(GLOB ${PROJECT_NAME}_headers *.h)
file(GLOB ${PROJECT_NAME}_sources *.cpp)

set(USE_QT true)

if(USE_QT)
    if(NOT DEFINED ENV{QTDIR})
        if(WIN32)
            set(QTDIR "c:/Qt/Qt5.5.0")
        else()
            set(QTDIR "~/Qt")
        endif()
    else()
        set(QTDIR $ENV{QTDIR})
    endif()

    set(CMAKE_PREFIX_PATH ${QTDIR})
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    find_program(QT_UIC_EXECUTABLE uic)
endif()

if(APPLE)
    # icon
    if(EXISTS "${PROJECT_NAME}.icns")
        # set how it shows up in the Info.plist file
        set(MACOSX_BUNDLE_ICON_FILE ${PROJECT_NAME}.icns)

        # set where in the bundle to put the icns file
        set_source_files_properties(${PROJECT_NAME}.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

        # include the icns file in the target
        set(${PROJECT_NAME}_sources ${${PROJECT_NAME}_sources} ${PROJECT_NAME}.icns)
    endif()

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${${PROJECT_NAME}_headers} ${${PROJECT_NAME}_sources} ${UI_HEADERS})
elseif(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${${PROJECT_NAME}_headers} ${${PROJECT_NAME}_sources} ${UI_HEADERS})
else()
    add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_headers} ${${PROJECT_NAME}_sources} ${UI_HEADERS})
endif()

target_link_libraries(${PROJECT_NAME} jcon)

if(USE_QT)
    find_package(Qt5 COMPONENTS Core)
    qt5_use_modules(${PROJECT_NAME} Core Network Test WebSockets Widgets)
endif()

if(APPLE)
    set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}")

    set(plugin_dest_dir ${PROJECT_NAME}.app/Contents/MacOS)
    set(APPS "\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app")

    install(CODE "
            include(BundleUtilities)
            fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")
            " COMPONENT Runtime)

else()
    install(TARGETS ${EXECUTABLE_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
endif()

if(MINGW)
    get_filename_component(MINGW_DIR ${CMAKE_C_COMPILER} PATH)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MINGW_DIR}/libgcc_s_dw2-1.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MINGW_DIR}/libstdc++-6.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

add_subdirectory(jcon)
