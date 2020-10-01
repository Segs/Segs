SET(QT_MISSING True)
# msvc only; mingw will need different logic
IF(MSVC)
    message("LocateQt5 was given a Qt5 path ${QT_GIVEN_PATH}")
    # look for user-registry pointing to qtcreator
    GET_FILENAME_COMPONENT(QT_BIN [HKEY_CURRENT_USER\\Software\\Classes\\Applications\\QtProject.QtCreator.cpp\\shell\\Open\\Command] PATH)
    if(${MSVC_VERSION} VERSION_LESS "1910")
        MESSAGE(FATAL_ERROR "SEGS requires visual studio 2017 to build")
    endif()
    # get root path so we can search for 5.3, 5.4, 5.5, etc
    get_filename_component(t_path ${QT_BIN} DIRECTORY)
    get_filename_component(t_path ${t_path} DIRECTORY)
    get_filename_component(QT_BIN ${t_path} DIRECTORY)

    LIST(GET QT_BIN 0 QT_BIN)
    FILE(GLOB QT_VERSIONS "${QT_BIN}/5.*")
    # find the latest qt version in QT_VERSIONS
    foreach(Ver ${QT_VERSIONS})
        if(NOT QT_VERSION)
            set(QT_VERSION ${Ver})
        endif()
        if(${Ver} VERSION_GREATER ${QT_VERSION})
            set(QT_VERSION ${Ver})
        endif()
    endforeach()

    # fix any double slashes which seem to be common
    STRING(REPLACE "//" "/"  QT_VERSION "${QT_VERSION}")

    # we only support 2017, and qt5 uses vs2015 files for 32 bit build
    SET(QT_MSVC "2015")
    # check for 64-bit os
    # may need to be removed for older compilers as it wasn't always offered
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
        SET(QT_MSVC "2017_64")
    endif()

    SET(QT_PATH "${QT_VERSION}/msvc${QT_MSVC}")
    if(NOT EXISTS ${QT_PATH})
        SET(QT_PATH "${QT_VERSION}/msvc2019_64")
    endif()
    if(QT_GIVEN_PATH)
        set(QT_PATH "${QT_GIVEN_PATH}")
    endif()
    if(EXISTS ${QT_PATH})
    SET(QT_MISSING False)
    file(TO_NATIVE_PATH "${QT_PATH}/bin" QT_BIN_DIR_WINDOWS)
        set(RUN_PATH "${QT_BIN_DIR_WINDOWS};${CMAKE_MSVCIDE_RUN_PATH}")
        list(REMOVE_DUPLICATES RUN_PATH)
        set(CMAKE_MSVCIDE_RUN_PATH "${RUN_PATH}" CACHE STATIC "MSVC IDE Run path" FORCE)
    endif()
ENDIF()

MACRO(TO_NATIVE_PATH PATH OUT)
        FILE(TO_NATIVE_PATH "${PATH}" "${OUT}")
        IF(MINGW)
                STRING(REPLACE "/" "\\" "${OUT}" "${${OUT}}")
        ENDIF(MINGW)
ENDMACRO(TO_NATIVE_PATH)
# use Qt_DIR approach so you can find Qt after cmake has been invoked
IF(NOT QT_MISSING)
    MESSAGE("-- Qt found: ${QT_PATH}")
    SET(Qt5_DIR "${QT_PATH}/lib/cmake/Qt5/")
    SET(Qt5Test_DIR "${QT_PATH}/lib/cmake/Qt5Test")
ENDIF()
find_package(Qt5 REQUIRED COMPONENTS Core)
get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(Qt5_BIN_DIR "${_qmake_executable}" DIRECTORY)
TO_NATIVE_PATH("${Qt5_BIN_DIR}" Qt5_BIN_DIR)

