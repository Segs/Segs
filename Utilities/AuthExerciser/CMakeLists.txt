find_package(Threads REQUIRED)

set(SOURCES main.cpp DummyClass.cpp)
set(HEADERS DummyClass.h)

add_executable(AuthExerciser ${SOURCES} ${HEADERS})
add_dependencies(AuthExerciser ace_IMP cereal_IMP) # used for Singleton stuff.
target_link_libraries(AuthExerciser PUBLIC Qt5::Core)

set_target_properties (AuthExerciser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities
)

TARGET_LINK_LIBRARIES(AuthExerciser PUBLIC
    Threads::Threads
    ${target_DEPENDS}
    AuthProtocol
    SEGS_Components
    ace_IMP
    ${CMAKE_DL_LIBS}
    Threads::Threads
)
install(TARGETS AuthExerciser
    DESTINATION deploy
)
if(MINGW)
    target_link_libraries(AuthExerciser PUBLIC ws2_32)
endif()

IF(WIN32)
    include(Windeployqt)
    windeployqt(AuthExerciser deploy)
ELSEIF(APPLE)
    include(Macdeployqt)
    macdeployqt(AuthExerciser deploy)
ENDIF()
