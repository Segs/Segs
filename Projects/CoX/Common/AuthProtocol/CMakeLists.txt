
# $Id:$
SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthConnection.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthPacket.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthPacketCodec.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthProtocol.cpp 
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthConnection.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthPacket.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthPacketCodec.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthProtocol.h
)
SET(target_INCLUDE_DIR "")
# add components here
# end of additional components
INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

#ADD_LIBRARY(AuthProtocol STATIC ${target_SOURCES})

SEGS_REGISTER_COMPONENT(AuthProtocol "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )