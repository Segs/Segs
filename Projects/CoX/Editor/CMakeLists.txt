set(SE_SDK_PATH ${PROJECT_SOURCE_DIR}/../SegsEngine_installdir)
if(EXISTS "${SE_SDK_PATH}/cmake/editor_engine_Targets.cmake")
    include(${SE_SDK_PATH}/cmake/editor_engine_Targets.cmake)
endif()


add_executable(SegsEditor main.cpp)
target_link_libraries(SegsEditor SegsEngine::editor_engine)
