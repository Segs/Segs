
SET(BUILD_ChampionsOnline_LoginServer TRUE CACHE BOOL "Build LoginServer?")
IF(BUILD_ChampionsOnline_LoginServer)
	ADD_SUBDIRECTORY(LoginServer)
ENDIF(BUILD_ChampionsOnline_LoginServer)
SET(BUILD_ChampionsOnline_TestServer TRUE CACHE BOOL "Build TestServer?")
IF(BUILD_ChampionsOnline_TestServer)
	ADD_SUBDIRECTORY(TestServer)
ENDIF(BUILD_ChampionsOnline_TestServer)