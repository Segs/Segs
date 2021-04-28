find_package(Qt5 COMPONENTS Core REQUIRED)

add_executable(binConverter
    binConverter.cpp
)
target_link_libraries(binConverter PUBLIC gameData cereal_IMP)
target_link_libraries(binConverter PUBLIC Qt5::Core)


# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES binConverter SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (binConverter PROPERTIES
    FOLDER Utilities
    RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities
)
