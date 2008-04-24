SET(UNICODE FALSE) # needs some serious thought
IF(UNICODE)
  ADD_DEFINITIONS(-D_UNICODE -DUNICODE)
ENDIF(UNICODE)

FIND_PACKAGE(wxWidgets REQUIRED base core)
FIND_PACKAGE(OGRE REQUIRED)
FIND_PACKAGE(ACE REQUIRED)
FIND_PACKAGE(Boost)

SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/stdwx.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/wxOgrePanel.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/stdwx.h
${CMAKE_CURRENT_SOURCE_DIR}/include/wxOgrePanel.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

SEGS_ADD_COMPONENT(Globals SOURCE)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
SET(target_INCLUDE_DIR ${target_INCLUDE_DIR}
    ${OGRE_path}
    ${ACE_path}
    ${Boost_INCLUDE_DIRS}
    ${wxWidgets_INCLUDE_DIRS}
)
SEGS_REGISTER_COMPONENT(OgrePanel "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

add_definitions( 
  -D_CRT_SECURE_NO_WARNINGS
  -D_LIB
  -D__WXDEBUG__
)

add_library( OgrePanel STATIC ${target_SOURCES} )