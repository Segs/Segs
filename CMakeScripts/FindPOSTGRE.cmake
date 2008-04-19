# Find the Postgre headers and library

IF (PGSQL_path AND PGSQL_lib)
    SET(PGSQL_FIND_QUIETLY TRUE)
ENDIF (PGSQL_path AND PGSQL_lib)

FIND_PATH(PGSQL_path libpq-fe.h
   /usr/local/include/postgresql
   /usr/include/postgresql
   /usr/include/pgsql
  ${MAIN_INCLUDE_PATH}
  ${MAIN_INCLUDE_PATH}/postgre
)

FIND_LIBRARY(PGSQL_lib 
	NAMES 
		libpq.so libpq.a libpq pq 
	PATHS 
		/usr/lib 
		/usr/local/lib 
		${MAIN_LIB_PATH}
	DOC "Path to PostgreSQL library"]
)

IF (PGSQL_path AND PGSQL_lib)
   SET(PGSQL_FOUND TRUE)
ELSE (PGSQL_path AND PGSQL_lib)
   SET(PGSQL_FOUND FALSE)
ENDIF (PGSQL_path AND PGSQL_lib)

IF (PGSQL_FOUND)
  IF (NOT PGSQL_FIND_QUIETLY)
    MESSAGE(STATUS "Found PGSQL: ${PGSQL_lib}")
  ENDIF (NOT PGSQL_FIND_QUIETLY)
ELSE (PGSQL_FOUND)
  IF (PGSQL_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could NOT find PGSQL")
  ENDIF (PGSQL_FIND_REQUIRED)
ENDIF (PGSQL_FOUND)

MARK_AS_ADVANCED(PGSQL_path PGSQL_lib)
