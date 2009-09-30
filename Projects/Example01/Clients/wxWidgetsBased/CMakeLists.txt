# I found simple solution to setting correct values for finding wxwidgets wxWidgets_LIB_DIR 
# should be set by hand to proper directory i.e. ..wxWidgets-2.8.4/lib/7_1/vc_lib or wxWidgets-2.8.4/lib/8_0/vc_lib

SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/generated/main.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/generated/LoginDialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/Avatar.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/generated/LoginDialog.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Avatar.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Example01.h
)
SET(target_INCLUDE_DIR 
	${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/generated
)

FIND_PACKAGE(wxWidgets REQUIRED base core)
FIND_PACKAGE(OGRE REQUIRED)
FIND_PACKAGE(ACE REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)
 
IF (wxWidgets_FOUND)
    IF(wxWidgets_INCLUDE_DIRS)
      INCLUDE_DIRECTORIES(${wxWidgets_INCLUDE_DIRS})
    ENDIF(wxWidgets_INCLUDE_DIRS)
    IF (wxWidgets_CXX_FLAGS)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${wxWidgets_CXX_FLAGS}")
    ENDIF(wxWidgets_CXX_FLAGS)
    IF(wxWidgets_DEFINITIONS)
      ADD_DEFINITIONS(${wxWidgets_DEFINITIONS})
    ENDIF(wxWidgets_DEFINITIONS)
ENDIF (wxWidgets_FOUND)

ADD_DEFINITIONS( 
  -D_CRT_SECURE_NO_WARNINGS
  -D_LIB
  -D__WXDEBUG__
)

SEGS_ADD_COMPONENT(Auth SOURCE)
LINK_DIRECTORIES(${OGRE_LIB_DIR})
SET(target_INCLUDE_DIR ${target_INCLUDE_DIR}
    ${OGRE_INCLUDE_DIR}
    ${ACE_path}
    ${Boost_INCLUDE_DIRS}
    ${wxWidgets_INCLUDE_DIRS}
)
INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

IF(WIN32)
	ADD_EXECUTABLE( segs_client_example_01 WIN32 ${target_SOURCES} )
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows") # strangely adding WIN32 didn't set /subsystem:windows
ELSE(WIN32)
	ADD_EXECUTABLE( segs_client_example_01 ${target_SOURCES} )
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(segs_client_example_01 segs_common 
						${ACE_lib} 
						${ZLIB_LIBRARIES}
						${CMAKE_THREAD_LIBS_INIT}
						${OGRE_LIBRARIES}
                        ${wxWidgets_LIBRARIES}) 
ADD_DEPENDENCIES(segs_client_example_01 authserver_lib segs_common gameserver_lib) 
