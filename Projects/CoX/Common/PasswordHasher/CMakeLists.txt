SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/PasswordHasher.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/PasswordHasher.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ACE_path})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

ADD_LIBRARY(password_hasher STATIC ${target_SOURCES})
set_target_properties(password_hasher PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})

qt5_use_modules(password_hasher LINK_PUBLIC Core)
ADD_DEPENDENCIES(password_hasher cereal_IMP CRUDP_Protocol)
