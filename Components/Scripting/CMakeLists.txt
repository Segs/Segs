SET(target_CPP
    ${CMAKE_CURRENT_SOURCE_DIR}/MRubyEngine.cpp
)
SET(target_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/MRubyEngine.h
)

SET(target_SOURCES
    ${target_CPP}
    ${target_INCLUDE}
)
SET(target_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MRuby_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

ADD_LIBRARY(MRuby ${target_SOURCES})
ADD_DEPENDENCIES(MRuby MRUBY_BUILD)
TARGET_LINK_LIBRARIES(MRuby ${MRuby_LIBRARIES})

SEGS_REGISTER_COMPONENT(MRuby "${target_INCLUDE_DIR}" ${target_CPP} ${target_INCLUDE} )

if(ENABLE_TESTS)
add_subdirectory(tests)
endif()
