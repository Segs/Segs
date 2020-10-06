SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/AuthEventFactory.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuthLink.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuthPacketCodec.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/AuthEventFactory.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthLink.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthOpcodes.h
${CMAKE_CURRENT_SOURCE_DIR}/AuthPacketCodec.h
)

SET(target_INCLUDE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/Events"
)

# add components here
# end of additional components

INCLUDE_DIRECTORIES(
${target_INCLUDE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}Events/
)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

ADD_LIBRARY(AuthProtocol STATIC ${target_SOURCES})
target_link_libraries(AuthProtocol PUBLIC Qt5::Core ace_IMP Auth_interface)
target_link_libraries(AuthProtocol INTERFACE SEGS_Components)
set_property(TARGET AuthProtocol PROPERTY POSITION_INDEPENDENT_CODE ON)
if(ENABLE_TESTS)
    add_subdirectory(UnitTests)
endif()
# Retain directory structure in visual studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${target_CPP} ${target_INCLUDE}) 
set_target_properties (AuthProtocol PROPERTIES
    FOLDER Common
)
