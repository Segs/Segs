set(SOURCES 
  generated/main.cpp
  generated/LoginDialog.cpp
)

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}/generated
)

add_definitions( 
  -D_CRT_SECURE_NO_WARNINGS
  -D_LIB
  -D__WXDEBUG__
)
ADD_DEFINITIONS(${wxWidgets_DEFINITIONS})
ADD_EXECUTABLE( segs_client_example_01 ${SOURCES} )
TARGET_LINK_LIBRARIES(segs_client_example_01 segs_common 
						${ACE_lib} 
						${ZLIB_LIBRARIES}
						${CMAKE_THREAD_LIBS_INIT}
                                                ${wxWidgets_LIBRARIES}) 
ADD_DEPENDENCIES(segs_client_example_01 authserver_lib segs_common gameserver_lib) 
