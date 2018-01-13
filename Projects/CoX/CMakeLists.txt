include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(Common)

ADD_SUBDIRECTORY(Common)
ADD_SUBDIRECTORY(Utilities)
ADD_SUBDIRECTORY(Servers)
ADD_SUBDIRECTORY(Clients)

# Add README, settings.cfg, and Starter Databases
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/README.md ${EXECUTABLE_OUTPUT_PATH}/README.md COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/settings.cfg ${EXECUTABLE_OUTPUT_PATH}/default_dbs/settings.cfg COPYONLY)
if(NOT EXISTS ${EXECUTABLE_OUTPUT_PATH}/settings.cfg)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/settings.cfg ${EXECUTABLE_OUTPUT_PATH}/settings.cfg COPYONLY)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/sqlite/segs ${EXECUTABLE_OUTPUT_PATH}/default_dbs/segs COPYONLY)
if(NOT EXISTS ${EXECUTABLE_OUTPUT_PATH}/segs)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/sqlite/segs ${EXECUTABLE_OUTPUT_PATH}/segs COPYONLY)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/sqlite/segs_game ${EXECUTABLE_OUTPUT_PATH}/default_dbs/segs_game COPYONLY)
if(NOT EXISTS ${EXECUTABLE_OUTPUT_PATH}/segs_game)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/sqlite/segs_game ${EXECUTABLE_OUTPUT_PATH}/segs_game COPYONLY)
endif()
