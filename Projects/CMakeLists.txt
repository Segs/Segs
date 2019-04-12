INCLUDE(CMakeDependentOption)
OPTION(BUILD_CoX "Should the CoX emulation be built?" ON )
OPTION(BUILD_Examples "Should the Example Projects be built?" OFF)

CMAKE_DEPENDENT_OPTION(BUILD_Example01 "Should the first example be built?" OFF "BUILD_Examples" OFF)
if(BUILD_CoX)
        ADD_SUBDIRECTORY(CoX)
endif()

IF(BUILD_Examples)
    IF(BUILD_Example01)
        ADD_SUBDIRECTORY(Example01)
    ENDIF(BUILD_Example01)
ENDIF(BUILD_Examples)

SET(BUILD_ChampionsOnline FALSE CACHE BOOL "Build ChampionsOnline?")
IF(BUILD_ChampionsOnline)
        ADD_SUBDIRECTORY(ChampionsOnline)
ENDIF(BUILD_ChampionsOnline)
