project(jcon)

set(QTDIR $ENV{QTDIR})
set(QT5_DIR $ENV{QTDIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH ${QTDIR})

file(GLOB ${PROJECT_NAME}_headers *.h)
file(GLOB ${PROJECT_NAME}_sources *.cpp)

#add_definitions(-DJCON_DLL)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_headers} ${${PROJECT_NAME}_sources})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${${PROJECT_NAME}_headers}")

find_package(Qt6 COMPONENTS  Core Network Test WebSockets)
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
		PUBLIC_HEADER DESTINATION include/jcon
)
target_compile_definitions(jcon PUBLIC NDEBUG)
target_link_libraries(jcon PUBLIC Qt6::Core Qt6::Network Qt6::WebSockets)
