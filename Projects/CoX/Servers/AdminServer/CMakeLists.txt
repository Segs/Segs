SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/AdminServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AdminDatabase.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AccountInfo.cpp
${CMAKE_CURRENT_SOURCE_DIR}/CharacterDatabase.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/CharacterDatabase.h
${CMAKE_CURRENT_SOURCE_DIR}/AccountInfo.h
${CMAKE_CURRENT_SOURCE_DIR}/AdminDatabase.h
${CMAKE_CURRENT_SOURCE_DIR}/AdminServer.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(target_DEPENDS )

SEGS_ADD_COMPONENT(cox_server_common LIB)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

ADD_LIBRARY(adminserver_lib STATIC ${target_SOURCES})

TARGET_LINK_LIBRARIES(adminserver_lib ${target_DEPENDS} sqlite_IMP)
ADD_DEPENDENCIES(adminserver_lib ${target_DEPENDS} sqlite_IMP)
qt5_use_modules(adminserver_lib Core)
SEGS_REGISTER_COMPONENT(adminserver_lib "${target_INCLUDE_DIR}" "${target_INCLUDE}" )
