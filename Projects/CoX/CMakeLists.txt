find_package(PostgreSQL) # PostgreSQL
if(PostgreSQL_FOUND)
    add_definitions(-DHAVE_POSTGRES)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

ADD_SUBDIRECTORY(Common)
ADD_SUBDIRECTORY(Utilities)
ADD_SUBDIRECTORY(Servers)
ADD_SUBDIRECTORY(Clients)

