OPTION(BUILD_CoX "Should the CoX emulation be built?" ON )
OPTION(BUILD_Examples "Should the Example Projects be built?" OFF)

IF(BUILD_CoX)
        ADD_SUBDIRECTORY(CoX)
ENDIF(BUILD_CoX)

IF(BUILD_Examples)
        SET(BUILD_Example01 FALSE CACHE BOOL "Should the first example be built?")
    IF(BUILD_Example01)
        ADD_SUBDIRECTORY(Example01)
    ENDIF(BUILD_Example01)
ENDIF(BUILD_Examples)



SET(BUILD_ChampionsOnline FALSE CACHE BOOL "Build ChampionsOnline?")
IF(BUILD_ChampionsOnline)
        ADD_SUBDIRECTORY(ChampionsOnline)
ENDIF(BUILD_ChampionsOnline)
