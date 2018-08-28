include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(Common)

ADD_SUBDIRECTORY(Common)
ADD_SUBDIRECTORY(Utilities)
ADD_SUBDIRECTORY(Servers)
ADD_SUBDIRECTORY(Clients)

# Add README, scripts, settings.cfg, and Starter Databases
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/README.md ${EXECUTABLE_OUTPUT_PATH}/README.md COPYONLY)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/motd.smlx DESTINATION ${EXECUTABLE_OUTPUT_PATH}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/test.smlx DESTINATION ${EXECUTABLE_OUTPUT_PATH}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/tutorial.smlx DESTINATION ${EXECUTABLE_OUTPUT_PATH}/scripts)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/settings_template.cfg ${EXECUTABLE_OUTPUT_PATH}/settings_template.cfg COPYONLY)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/mysql DESTINATION ${EXECUTABLE_OUTPUT_PATH}/default_dbs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/pgsql DESTINATION ${EXECUTABLE_OUTPUT_PATH}/default_dbs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/sqlite DESTINATION ${EXECUTABLE_OUTPUT_PATH}/default_dbs)
