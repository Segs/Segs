SET(CMAKE_CXX_STANDARD 11)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -DNOMINMAX)
    ADD_DEFINITIONS(/W4)
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")
ENDIF()
find_package(Qt5Core)

set(tool_SRC
    main.cpp
)
add_executable(piggtool ${tool_SRC})
target_link_libraries(piggtool Qt5::Core)
# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES piggtool SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (piggtool PROPERTIES
    FOLDER Utilities
    RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities
)
