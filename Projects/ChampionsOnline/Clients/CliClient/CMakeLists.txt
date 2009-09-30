# $Id:$
SET(target_CPP
)
SET(target_INCLUDE
)
SET(target_INCLUDE_DIR "")
# add components here
# end of additional components
INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)
#ADD_LIBRARY(ChampionsOnline_CliClient STATIC ${target_SOURCES})
SEGS_REGISTER_COMPONENT(ChampionsOnline_CliClient ${target_INCLUDE_DIR} ${target_CPP} ${target_INCLUDE} )
