
add_executable(AuthExerciser main.cpp)
add_dependencies(AuthExerciser ace_IMP cereal_IMP) # used for Singleton stuff.
qt5_use_modules(AuthExerciser Core)

TARGET_LINK_LIBRARIES(AuthExerciser
	${target_DEPENDS} 
	SEGS_Components
	AuthProtocol
	${CMAKE_THREAD_LIBS_INIT}
	ace_IMP
	${CMAKE_DL_LIBS}
)
	