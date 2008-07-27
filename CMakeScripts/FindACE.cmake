# Find the ACE headers and library

IF (ACE_path AND ACE_lib)
    SET(ACE_FIND_QUIETLY TRUE)
ENDIF (ACE_path AND ACE_lib)

FIND_PATH(ACE_path ace/ACE.h
  /usr/local/include/ace
  /usr/include/ace
  ${MAIN_INCLUDE_PATH}
  ${MAIN_INCLUDE_PATH}/ace
)

FIND_LIBRARY(ACE_lib 
	NAMES ACE ACEd libACE
	PATHS 
		/usr/lib 
		/usr/local/lib 
		/usr/lib /ACEd
		/usr/local/lib/ACEd 
		${MAIN_LIB_PATH}
		${MAIN_LIB_PATH}/ACE
		${MAIN_LIB_PATH}/ACEd
		${ACE_path}/lib
	DOC "Path to ACE framework library"]
)

IF (ACE_path AND ACE_lib)
   SET(ACE_FOUND TRUE)
ELSE (ACE_path AND ACE_lib)
   SET(ACE_FOUND FALSE)
ENDIF (ACE_path AND ACE_lib)

IF (ACE_FOUND)
  IF (NOT ACE_FIND_QUIETLY)
    MESSAGE(STATUS "Found ACE: ${ACE_lib}")
  ENDIF (NOT ACE_FIND_QUIETLY)
ELSE (ACE_FOUND)
  IF (ACE_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could NOT find ACE")
  ENDIF (ACE_FIND_REQUIRED)
ENDIF (ACE_FOUND)

MARK_AS_ADVANCED(ACE_path ACE_lib)
