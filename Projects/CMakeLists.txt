# $Id: CMakeLists.txt 306 2007-01-23 12:16:25Z nemerle $
SET(BUILD_CoX TRUE CACHE BOOL "Should the CoX emulation be built?")
IF(BUILD_CoX)
	ADD_SUBDIRECTORY(CoX)
ENDIF(BUILD_CoX)
