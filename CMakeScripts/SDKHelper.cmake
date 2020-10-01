# the following code is based on BSF's HelperMethods.cmake

set(SE_BINARY_DEP_WEBSITE "https://github.com/Segs/SegsEngine/releases/download")
#v4.0.0.9-alpha/SegsEngine_4.0.0.9.7z
function(update_binary_deps DEP_PREFIX DEP_NAME DEP_FOLDER DEP_VERSION)
    if(NOT WIN32)
        return()
    endif()
    # Clean and create a temporary folder
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/Temp)

    set(BINARY_DEPENDENCIES_URL ${SE_BINARY_DEP_WEBSITE}/v${DEP_VERSION}-alpha/${DEP_PREFIX}_${DEP_VERSION}.7z)
    file(DOWNLOAD ${BINARY_DEPENDENCIES_URL} ${PROJECT_SOURCE_DIR}/Temp/Dependencies.7z
        SHOW_PROGRESS
        STATUS DOWNLOAD_STATUS)

    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    if(NOT STATUS_CODE EQUAL 0)
        message(FATAL_ERROR "Binary dependencies failed to download from URL: ${BINARY_DEPENDENCIES_URL}")
    endif()

    message(STATUS "Extracting files. Please wait...")
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/Temp/unpack)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${PROJECT_SOURCE_DIR}/Temp/Dependencies.7z
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Temp/unpack
    )
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${DEP_FOLDER}/bin)
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${DEP_FOLDER}/cmake)
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${DEP_FOLDER}/include)
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${DEP_FOLDER}/lib)
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${DEP_FOLDER})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Temp/unpack ${DEP_FOLDER})
#    execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)

endfunction()

function(check_and_update_binary_deps DEP_PREFIX DEP_NAME DEP_FOLDER DEP_VERSION)
    set(BUILTIN_DEP_VERSION_FILE ${DEP_FOLDER}/.version)
    message("DEP_VERSION ${BUILTIN_DEP_VERSION_FILE}")
    if(NOT EXISTS ${BUILTIN_DEP_VERSION_FILE})
        message(STATUS "Binary dependencies for '${DEP_PREFIX}' are missing. Downloading package...")
        update_binary_deps(${DEP_PREFIX} ${DEP_NAME} ${DEP_FOLDER} ${DEP_VERSION})
    else()
        file (STRINGS ${BUILTIN_DEP_VERSION_FILE} CURRENT_DEP_VERSION)
        if(${DEP_VERSION} VERSION_GREATER ${CURRENT_DEP_VERSION})
            message(STATUS "Your precomiled dependencies package for '${DEP_PREFIX}' is out of date. Downloading latest package...")
            update_binary_deps(${DEP_PREFIX} ${DEP_NAME} ${DEP_FOLDER} ${DEP_VERSION})
        endif()
    endif()
endfunction()
