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
