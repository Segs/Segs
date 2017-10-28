include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(Common)

ADD_SUBDIRECTORY(Common)
ADD_SUBDIRECTORY(Utilities)
ADD_SUBDIRECTORY(Servers)
ADD_SUBDIRECTORY(Clients)

# Add README, settings.cfg, and Starter Databases
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/README.md ${EXECUTABLE_OUTPUT_PATH}/README.md COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/settings.cfg ${EXECUTABLE_OUTPUT_PATH}/settings.cfg COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/sqlite/segs ${EXECUTABLE_OUTPUT_PATH}/segs COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/sqlite/segs ${EXECUTABLE_OUTPUT_PATH}/segs_game COPYONLY)