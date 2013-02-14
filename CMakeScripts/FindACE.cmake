# Find the ACE headers and library

SET(ACE_debug_lib FALSE CACHE BOOL "Use debug version of ACE")
SET(ACELIB_extension "")

# TODO: Handle other 'debug-like' configurations here
IF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    SET(ACE_debug_lib TRUE)
ENDIF()

IF(MSVC)
    IF(ACE_debug_lib)
        SET(ACELIB_extension "d")
    ENDIF()
ENDIF()
IF (ACE_path AND ACE_lib_path)
    SET(ACE_FIND_QUIETLY TRUE)
ENDIF (ACE_path AND ACE_lib_path)

FIND_PATH(ACE_path ace/ACE.h
  /usr/local/include/ace
  /usr/include/ace
  ${MAIN_INCLUDE_PATH}
  ${MAIN_INCLUDE_PATH}/ace
)

FIND_PATH(ACE_lib_path NAMES ACE.lib libACE.so
    PATHS
        /usr/lib
        /usr/local/lib
        ${MAIN_LIB_PATH}
        ${ACE_path}/lib
    DOC "Path to ACE framework library"
)

IF (ACE_path AND ACE_lib_path)
   SET(ACE_FOUND TRUE)
ELSE (ACE_path AND ACE_lib_path)
   SET(ACE_FOUND FALSE)
ENDIF (ACE_path AND ACE_lib_path)

IF (ACE_FOUND)
  IF (NOT ACE_FIND_QUIETLY)
    MESSAGE(STATUS "Found ACE: ${ACE_lib_path}")
  ENDIF (NOT ACE_FIND_QUIETLY)
ELSE (ACE_FOUND)
  IF (ACE_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could NOT find ACE")
  ENDIF (ACE_FIND_REQUIRED)
ENDIF (ACE_FOUND)

MARK_AS_ADVANCED(ACE_path ACE_lib_path)
LINK_DIRECTORIES(${ACE_lib_path})
FUNCTION(
ACE_ADD_LIBRARIES target)
    FOREACH(libname ${ARGN})
        TARGET_LINK_LIBRARIES(${target}
            optimized
                ${libname}
            debug
                ${libname}${ACELIB_extension}
        )
    ENDFOREACH()
ENDFUNCTION()
