if(ENABLE_SCRIPTING_ENGINE_MRUBY)
    ADD_SUBDIRECTORY(Scripting)
endif()

SET(target_CPP
    BitStream.cpp
    Buffer.cpp
    ConfigExtension.cpp
    EventProcessor.cpp
    SEGSEvent.cpp
    SEGSTimer.cpp
    FixedPointValue.cpp
    Settings.cpp
    Logging.cpp
)
SET(target_INCLUDE
    BitStream.h
    Buffer.h
    ConfigExtension.h
    EventProcessor.h
    SEGSEvent.h
    LinkLevelEvent.h
    SEGSTimer.h
    FixedPointValue.h
    Settings.h
    Logging.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

add_library(SEGS_Components STATIC ${target_SOURCES} )
add_dependencies(SEGS_Components ACE_BUILD)

target_link_libraries(SEGS_Components Qt5::Core)





