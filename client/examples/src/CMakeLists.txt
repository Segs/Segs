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

ADD_LIBRARY( wxTools STATIC ${SOURCES} )
