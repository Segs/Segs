INCLUDE(CMakeDependentOption)
OPTION(BUILD_CoX "Should the CoX emulation be built?" ON )

if(BUILD_CoX)
        ADD_SUBDIRECTORY(CoX)
endif()
