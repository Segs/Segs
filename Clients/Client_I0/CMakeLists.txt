CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

PROJECT(CoH_Patch)
IF(MSVC)
    add_definitions( -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -DNOMINMAX -D_USE_MATH_DEFINES)
    if (MSVC_VERSION GREATER_EQUAL "1900")
        include(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
        if (_cpp_latest_flag_supported)
            add_compile_options("/std:c++latest")
        endif()
    endif()
ELSE()#-Wduplicated-cond -Wduplicated-branches -Wnull-dereference
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wuseless-cast -Wshadow -std=c++11"  )
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../../../CMakeScripts;${CMAKE_MODULE_PATH})
MESSAGE(${CMAKE_SOURCE_DIR}/../../../../CMakeScripts)
include(3rdparty_support)
set(ThirdParty_Install_Dir ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/built)

add_subdirectory(3rd_party)
find_package(Threads REQUIRED)
link_directories(
    ${ThirdParty_Install_Dir}/lib
    ../patch_tools
)
include_directories(SYSTEM
    ${ThirdParty_Install_Dir}/include
    ${CMAKE_SOURCE_DIR}/../../../../3rd_party # glm is located there
)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    include
)
set(DLL_SOURCES
    3rd_party/mikktspace/mikktspace.c
    3rd_party/mikktspace/mikktspace.h
    include/GameState.h
    renderer/Model.h
    renderer/ModelCache.cpp
    renderer/ModelCache.h
    renderer/PBufferRenderer.cpp
    renderer/PBufferRenderer.h
    renderer/RendererUtils.cpp
    renderer/RendererUtils.h
    renderer/RenderShadow.cpp
    renderer/RenderShadow.h
    renderer/RendererState.cpp
    renderer/RendererState.h
    renderer/RenderModel.cpp
    renderer/RenderModel.h
    renderer/RenderBonedModel.cpp
    renderer/RenderBonedModel.h
    renderer/RenderParticles.cpp
    renderer/RenderParticles.h
    renderer/RenderSprites.cpp
    renderer/RenderSprites.h
    renderer/RenderTree.cpp
    renderer/RenderTree.h
    renderer/RenderTricks.cpp
    renderer/RenderTricks.h
    renderer/ShaderProgramCache.cpp
    renderer/ShaderProgramCache.h
    renderer/Texture.h
    renderer/Texture.cpp
    renderer/TextureGenerator.h
    renderer/TextureGenerator.cpp
    graphics/GroupDraw.cpp
    graphics/GroupDraw.h
    graphics/gfx.cpp
    graphics/gfx.h
    graphics/FontManager.cpp
    graphics/FontManager.h
    graphics/SeqGraphics.cpp
    graphics/SeqGraphics.h
    entity/EntClient.cpp
    entity/EntClient.h
    entity/entityDebug.cpp
    entity/entityDebug.h
    utils/helpers.cpp
    utils/helpers.h
)
set(SOURCES
    ${DLL_SOURCES}
    utils/dll_patcher.cpp
    patch_all_the_things.cpp
    dll_core.cpp
)
add_library(cohdll SHARED ${SOURCES})
# we link to the imp lib for the exe
target_link_libraries(cohdll CityOfX glew_IMP Threads::Threads)
# enforce the dll prefix.
SET_TARGET_PROPERTIES(cohdll PROPERTIES PREFIX "lib"
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES cohdll SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (cohdll PROPERTIES
    FOLDER Projects/CoX/Clients
)
