#------------------------------------------------------------------------------
# CPACK
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_VENDOR "Awesome SEGS peeps")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CONTACT "Nemerle <nemerle@segs.io>")
set(HOMEPAGE "https://github.com/Segs/Segs")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "SEGS - Super Entity Game Server")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.md")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/out")

set(CPACK_PACKAGE_EXECUTABLES "authserver;SEGS auth server;SEGSAdmin;SEGS Admin tool")
#####set(CPACK_CREATE_DESKTOP_LINKS "authserver")
set(CPACK_STRIP_FILES TRUE)

if(UNIX)
  set(CPACK_INSTALL_PREFIX local/)
  set(CPACK_BIN_DIR ${CPACK_INSTALL_PREFIX}/bin)
  set(CPACK_GENERATOR "DEB;TBZ2;TXZ")
  message(STATUS "Package generation - UNIX")
  message(STATUS "   + DEB                                  YES ")
  message(STATUS "   + TBZ2                                 YES ")
  message(STATUS "   + TXZ                                  YES ")
  message(STATUS "   + RPM                                  NO ")

  set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)
  set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${HOMEPAGE}")

  install(FILES ${PROJECT_SOURCE_DIR}/LICENSE.md
    DESTINATION local/share/doc/${PROJECT_NAME}/
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    RENAME copyright
  )

  install(FILES ${PROJECT_SOURCE_DIR}/Projects/CoX/Data/settings_template.cfg
    DESTINATION local/share/doc/${PROJECT_NAME}/
  )
endif()

include(CPack)
