SET(target_CPP
    basic_tests.cpp
)
SET(target_INCLUDE
)

SET(target_SOURCES
    ${target_CPP}
    ${target_INCLUDE}
)
SET(target_INCLUDE_DIR
    ${MRuby_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

ADD_EXECUTABLE(scripting_tests ${target_SOURCES})
ADD_DEPENDENCIES(scripting_tests GMOCK_BUILD)
TARGET_LINK_LIBRARIES(scripting_tests MRuby ${MRuby_LIBRARIES} ${GTest_LIBRARIES} ${ACE_LIBRARIES})





