SET(CMAKE_CXX_STANDARD 11)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -DNOMINMAX)
    ADD_DEFINITIONS(/W4)
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")
ENDIF()
find_package(Qt5 COMPONENTS Core Sql REQUIRED)

set(tool_SRC
    main.cpp
)
add_executable(dbtool ${tool_SRC})
qt5_use_modules(dbtool Core)
target_link_libraries(dbtool Qt5::Core Qt5::Sql)
target_link_libraries(dbtool password_hasher SEGS_Components)
