
# $Id:$
SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthConnection.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthPacket.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthPacketCodec.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthProtocol.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/packets/authserverlist.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthConnection.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthOpcodes.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthPacket.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthPacketCodec.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AuthProtocol.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Packets/authversion.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Packets/authlogin.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Packets/authloginresponse.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Packets/authrequestserverlist.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Packets/authserverlist.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Packets/authselectserver.h
${CMAKE_CURRENT_SOURCE_DIR}/include/Packets/authselectserverresponse.h
)
SET(target_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include/Packets")
# add components here
# end of additional components

INCLUDE_DIRECTORIES(
${target_INCLUDE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/include/Packets
)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

#ADD_LIBRARY(AuthProtocol STATIC ${target_SOURCES})

SEGS_REGISTER_COMPONENT(AuthProtocol "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )