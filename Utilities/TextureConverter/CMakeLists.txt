set(SRCS main.cpp)
add_executable(texConverter ${SRCS})
target_link_libraries(texConverter PRIVATE cereal_IMP)
target_link_libraries(texConverter PUBLIC Qt5::Core)
install(TARGETS texConverter
    DESTINATION deploy
)
# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES texConverter SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (texConverter PROPERTIES
    FOLDER Utilities
    RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities
)
