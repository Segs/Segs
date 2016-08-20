find_package(PostgreSQL) # PostgreSQL
if(PostgreSQL_FOUND)
    add_definitions(-DHAVE_POSTGRES)
endif()

ADD_SUBDIRECTORY(Common)
ADD_SUBDIRECTORY(Utilities)
ADD_SUBDIRECTORY(Servers)
ADD_SUBDIRECTORY(Clients)

