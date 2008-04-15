# $Id: CMakeLists.txt 306 2007-01-23 12:16:25Z nemerle $
SET(BUILD_CoX TRUE CACHE BOOL "Should the CoX emulation be built?")
SET(BUILD_Examples TRUE CACHE BOOL "Should the Example Projects be built?")

IF(BUILD_CoX)
	ADD_SUBDIRECTORY(CoX)
ENDIF(BUILD_CoX)

IF(BUILD_Examples)
	SET(BUILD_Example01 TRUE CACHE BOOL "Should the first example be built?")
    IF(BUILD_Example01)
        ADD_SUBDIRECTORY(Example01)
    ENDIF(BUILD_Example01)
ENDIF(BUILD_Examples)


