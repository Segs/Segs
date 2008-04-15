# $Id: CMakeLists.txt 301 2006-12-26 15:50:44Z nemerle $
SET(target_CPP
)
SET(target_INCLUDE
)

SET(target_INCLUDE_DIR "")

SEGS_ADD_COMPONENT(CRUDP_Protocol SOURCE)
SEGS_ADD_COMPONENT(ConfigExtensions SOURCE)
INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

ADD_LIBRARY(segs_common STATIC ${target_SOURCES})
SEGS_REGISTER_COMPONENT(segs_common "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )