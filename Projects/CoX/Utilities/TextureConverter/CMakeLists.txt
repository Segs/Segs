set(SRCS main.cpp)
add_executable(texConverter ${SRCS})
target_link_libraries(texConverter cereal_IMP)
qt5_use_modules(texConverter Core)
install(TARGETS texConverter
    DESTINATION deploy
)
