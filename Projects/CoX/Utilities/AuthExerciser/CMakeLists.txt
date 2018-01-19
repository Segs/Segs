set(SOURCES main.cpp DummyClass.cpp)
set(HEADERS DummyClass.h)
add_executable(AuthExerciser ${SOURCES} ${HEADERS})
add_dependencies(AuthExerciser ace_IMP cereal_IMP) # used for Singleton stuff.
qt5_use_modules(AuthExerciser Core)

TARGET_LINK_LIBRARIES(AuthExerciser
        ${target_DEPENDS}
        AuthProtocol
        SEGS_Components
	${CMAKE_THREAD_LIBS_INIT}
	ace_IMP
	${CMAKE_DL_LIBS}
)
install(TARGETS AuthExerciser
    DESTINATION deploy
)
if(MINGW)
    target_link_libraries(AuthExerciser ws2_32)
endif()
IF(WIN32)
    include(Windeployqt)
    windeployqt(AuthExerciser deploy)
ENDIF()
