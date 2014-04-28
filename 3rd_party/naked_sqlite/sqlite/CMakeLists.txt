SET(SQLite_INCLUDES
    sqlite3.h
    sqlite3ext.h
)

add_library(sqlite STATIC sqlite3.c)
install(TARGETS sqlite
        RUNTIME DESTINATION lib
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
)
install(FILES ${SQLite_INCLUDES} DESTINATION include/sqlite)

# ----- build sqlite3 utility 
find_package(Threads REQUIRED)
include_directories(${Threads_INCLUDE_DIR})
add_executable(sqlite3 shell.c)
target_link_libraries(sqlite3 sqlite)
IF(UNIX)
    TARGET_LINK_LIBRARIES(sqlite3 ${CMAKE_THREAD_LIBS_INIT} dl)
ENDIF()
# install built utility:
install(TARGETS sqlite3
        RUNTIME DESTINATION bin)
