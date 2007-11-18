set(SOURCES 
  stdwx.cpp
  wxOgrePanel.cpp
)

include_directories( 
)

add_definitions( 
  -D_CRT_SECURE_NO_WARNINGS
  -D_LIB
  -D__WXDEBUG__
)

add_library( wxTools STATIC ${SOURCES} )
