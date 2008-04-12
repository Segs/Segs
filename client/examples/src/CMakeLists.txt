set(SOURCES 
  generated/main.cpp
  generated/LoginDialog.cpp
  Avatar.cpp
)

INCLUDE_DIRECTORIES( 
  ${CMAKE_CURRENT_SOURCE_DIR}/generated
)

ADD_DEFINITIONS( 
  -D_CRT_SECURE_NO_WARNINGS
  -D_LIB
  -D__WXDEBUG__
)
IF(WIN32)
	ADD_EXECUTABLE( segs_client_example_01 WIN32 ${SOURCES} )
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows") # strangely adding WIN32 didn't set /subsystem:windows
ELSE(WIN32)
	ADD_EXECUTABLE( segs_client_example_01 ${SOURCES} )
ENDIF(WIN32)
TARGET_LINK_LIBRARIES(segs_client_example_01 segs_common 
						${ACE_lib} 
						${ZLIB_LIBRARIES}
						${CMAKE_THREAD_LIBS_INIT}
                        ${wxWidgets_LIBRARIES}) 
ADD_DEPENDENCIES(segs_client_example_01 authserver_lib segs_common gameserver_lib) 
