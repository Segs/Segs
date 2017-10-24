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
SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
ADD_LIBRARY(adminserver_lib STATIC ${target_SOURCES})
TARGET_LINK_LIBRARIES(adminserver_lib PUBLIC cox_server_common network_structures)
target_include_directories(adminserver_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
get_target_property(zxx adminserver_lib INCLUDE_DIRECTORIES)
message(${zxx})

qt5_use_modules(adminserver_lib LINK_PUBLIC Core)
