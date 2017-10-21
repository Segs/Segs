cmake_minimum_required(VERSION 2.8)

project(lua)

set(lua_library
  lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c
  lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c
  ltable.c ltm.c lundump.c lvm.c lzio.c
  lauxlib.c lbaselib.c ldblib.c lmathlib.c 
  # liolib.c - do not expose fillesystem access to the interpreter 
  # loslib.c - do not expose OS functions to the interpreter 
  # loadlib.c - do not expose shared library loading functions to the interpreter 
  ltablib.c lstrlib.c  linit.c
  lbitlib.c lcorolib.c lutf8lib.c
)
set(include_files
  src/lua.h
  src/luaconf.h
  src/lualib.h
  src/lauxlib.h
  src/lua.hpp
)
if(UNIX)
add_definitions(-DLUA_USE_LINUX)
endif()

foreach(s ${lua_library})
    set(lua_LIBRARY ${lua_LIBRARY} src/${s})
endforeach()

add_library(lua STATIC ${lua_LIBRARY})
install(TARGETS lua EXPORT lua-targets
            RUNTIME DESTINATION bin
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            INCLUDES DESTINATION include
)
install(EXPORT lua-targets DESTINATION lib/lua)
install(FILES ${include_files} DESTINATION include/lua )


