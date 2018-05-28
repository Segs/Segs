set(SRCS main.cpp)
add_executable(texConverter ${SRCS})
target_link_libraries(texConverter PRIVATE cereal_IMP)
target_link_libraries(texConverter PUBLIC Qt5::Core)
install(TARGETS texConverter
    DESTINATION deploy
)
