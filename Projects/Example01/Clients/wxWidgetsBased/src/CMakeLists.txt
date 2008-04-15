SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/generated/main.cpp
${CMAKE_CURRENT_SOURCE_DIR}/generated/LoginDialog.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Avatar.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/generated/LoginDialog.h
)

SET(target_INCLUDE_DIR 
	${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/generated
)

ADD_DEFINITIONS( 
  -D_CRT_SECURE_NO_WARNINGS
  -D_LIB
  -D__WXDEBUG__
)

SEGS_ADD_COMPONENT(CRUDP_Protocol SOURCE)
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
                        ${wxWidgets_LIBRARIES}) 
ADD_DEPENDENCIES(segs_client_example_01 authserver_lib segs_common gameserver_lib) 
