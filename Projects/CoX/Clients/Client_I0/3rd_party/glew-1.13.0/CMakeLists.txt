if ( NOT DEFINED CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release CACHE STRING "Build type" )
endif ()

project (glew)

cmake_minimum_required (VERSION 2.8)

if (COMMAND cmake_policy)
  cmake_policy (SET CMP0003 NEW)
endif()

set(CMAKE_DEBUG_POSTFIX d)

option (BUILD_UTILS "utilities" ON)

set (GLEW_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# get version from config/version
file (STRINGS ${GLEW_DIR}/config/version  _VERSION_MAJOR_STRING REGEX "GLEW_MAJOR[ ]*=[ ]*[0-9]+.*")
string (REGEX REPLACE "GLEW_MAJOR[ ]*=[ ]*([0-9]+)" "\\1" CPACK_PACKAGE_VERSION_MAJOR ${_VERSION_MAJOR_STRING})
file (STRINGS ${GLEW_DIR}/config/version  _VERSION_MINOR_STRING REGEX "GLEW_MINOR[ ]*=[ ]*[0-9]+.*")
string (REGEX REPLACE "GLEW_MINOR[ ]*=[ ]*([0-9]+)" "\\1" CPACK_PACKAGE_VERSION_MINOR ${_VERSION_MINOR_STRING})
file (STRINGS ${GLEW_DIR}/config/version  _VERSION_PATCH_STRING REGEX "GLEW_MICRO[ ]*=[ ]*[0-9]+.*")
string (REGEX REPLACE "GLEW_MICRO[ ]*=[ ]*([0-9]+)" "\\1" CPACK_PACKAGE_VERSION_PATCH ${_VERSION_PATCH_STRING})
set (GLEW_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (GLEW_LIB_NAME GLEW)

find_package (OpenGL REQUIRED)
set (GLEW_LIBRARIES ${OPENGL_LIBRARIES})

add_definitions (-DGLEW_NO_GLU)

include_directories (${GLEW_DIR}/include)

add_library (glew_s STATIC ${GLEW_DIR}/src/glew.c)
set_target_properties (glew_s PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC" OUTPUT_NAME "${GLEW_LIB_NAME}" PREFIX lib)
target_link_libraries (glew_s ${GLEW_LIBRARIES})

if(CMAKE_VERSION VERSION_LESS 2.8.12)
  set(MAYBE_EXPORT "")
else()
  target_compile_definitions(glew_s INTERFACE "GLEW_STATIC")
  foreach(t glew_s)
    target_include_directories(${t} PUBLIC $<INSTALL_INTERFACE:include>)
  endforeach()
  set(MAYBE_EXPORT EXPORT glew-targets)
endif()

set(targets_to_install "")
list(APPEND targets_to_install glew_s)

install ( TARGETS ${targets_to_install}
          ${MAYBE_EXPORT}
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib${LIB_SUFFIX}
          ARCHIVE DESTINATION lib${LIB_SUFFIX}
)

set (prefix ${CMAKE_INSTALL_PREFIX})
set (exec_prefix \${prefix})
set (libdir \${prefix}/lib)
set (includedir \${prefix}/include)
set (includedir \${prefix}/include)
set (version ${GLEW_VERSION})
set (libname ${GLEW_LIB_NAME})
set (cflags)
set (requireslib glu)
configure_file (${GLEW_DIR}/glew.pc.in ${GLEW_DIR}/glew.pc @ONLY)

install(FILES ${GLEW_DIR}/glew.pc
        DESTINATION lib/pkgconfig
)

install (FILES
    ${GLEW_DIR}/include/GL/wglew.h
    ${GLEW_DIR}/include/GL/glew.h
    ${GLEW_DIR}/include/GL/glxew.h
    DESTINATION include/GL)

if(MAYBE_EXPORT)
  install(EXPORT glew-targets DESTINATION lib/cmake/glew
    NAMESPACE GLEW::)
  install(FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/glew-config.cmake
      ${CMAKE_CURRENT_SOURCE_DIR}/CopyImportedTargetProperties.cmake
    DESTINATION lib/cmake/glew)
endif()
