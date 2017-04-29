
SET(BUILD_ChampionsOnline_GuiClient TRUE CACHE BOOL "Build GuiClient?")
IF(BUILD_ChampionsOnline_GuiClient)
	ADD_SUBDIRECTORY(GuiClient)
ENDIF(BUILD_ChampionsOnline_GuiClient)
SET(BUILD_ChampionsOnline_CliClient TRUE CACHE BOOL "Build CliClient?")
IF(BUILD_ChampionsOnline_CliClient)
	ADD_SUBDIRECTORY(CliClient)
ENDIF(BUILD_ChampionsOnline_CliClient)