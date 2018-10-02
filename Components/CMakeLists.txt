if(ENABLE_SCRIPTING_ENGINE_MRUBY)
    ADD_SUBDIRECTORY(Scripting)
endif()

SET(target_CPP
    serialization_types.cpp
    BitStream.cpp
    Buffer.cpp
    ConfigExtension.cpp
    EventProcessor.cpp
    SEGSEvent.cpp
    SEGSEventFactory.cpp
    SEGSTimer.cpp
    FixedPointValue.cpp
    Settings.cpp
    Logging.cpp
    PasswordHasher.cpp
    TimeHelpers.cpp
)
SET(target_INCLUDE
    Colors.h
    serialization_common.h
    serialization_types.h
    BitStream.h
    Buffer.h
    ConfigExtension.h
    EventProcessor.h
    SEGSEvent.h
    SEGSEventFactory.h
    LinkLevelEvent.h
    TimeEvent.h
    SEGSTimer.h
    FixedPointValue.h
    Settings.h
    Logging.h
    PasswordHasher.h
    TimeHelpers.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
scan_event_types(Common ${CMAKE_CURRENT_SOURCE_DIR})
add_event_sources(Common target_SOURCES)

add_library(SEGS_Components STATIC ${target_SOURCES} )
add_dependencies(SEGS_Components ACE_BUILD)

target_link_libraries(SEGS_Components PUBLIC Qt5::Core)
target_link_libraries(SEGS_Components INTERFACE ace_IMP)
IF(MINGW)
    target_link_libraries(SEGS_Components INTERFACE ws2_32)
ENDIF()

if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()




