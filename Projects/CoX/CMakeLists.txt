include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(Common)

ADD_SUBDIRECTORY(Common)
ADD_SUBDIRECTORY(Utilities)
ADD_SUBDIRECTORY(Servers)
ADD_SUBDIRECTORY(Clients)

# Add README, settings.cfg, and Starter Databases
if(EXISTS ${EXECUTABLE_OUTPUT_PATH}/README.md)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/README.md ${EXECUTABLE_OUTPUT_PATH}/defaults/README.md COPYONLY)
else
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/README.md ${EXECUTABLE_OUTPUT_PATH}/README.md COPYONLY)
endif

if(EXISTS ${EXECUTABLE_OUTPUT_PATH}/settings.cfg)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/settings.cfg ${EXECUTABLE_OUTPUT_PATH}/defaults/settings.cfg COPYONLY)
else
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/settings.cfg ${EXECUTABLE_OUTPUT_PATH}/settings.cfg COPYONLY)
endif

if(EXISTS ${EXECUTABLE_OUTPUT_PATH}/segs)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/sqlite/segs ${EXECUTABLE_OUTPUT_PATH}/defaults/segs COPYONLY)
else
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/sqlite/segs ${EXECUTABLE_OUTPUT_PATH}/segs COPYONLY)
endif

if(EXISTS ${EXECUTABLE_OUTPUT_PATH}/segs_game)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/sqlite/segs_game ${EXECUTABLE_OUTPUT_PATH}/defaults/segs_game COPYONLY)
else
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/sqlite/segs_game ${EXECUTABLE_OUTPUT_PATH}/segs_game COPYONLY)
endif
