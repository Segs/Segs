include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(Common)

ADD_SUBDIRECTORY(Common)
ADD_SUBDIRECTORY(Utilities)
ADD_SUBDIRECTORY(Servers)
ADD_SUBDIRECTORY(Clients)

# Add README, scripts, settings.cfg, and Starter Databases
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/README.md ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/README.md COPYONLY)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/motd.smlx DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/test.smlx DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/tutorial.smlx DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/global.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Encounter_Manager.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/ES_OL_Functions.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Global_NPC_Extras.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Global_Persistents.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/spawners.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Universal_Spawns.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)

#Outbreak
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_00_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_00_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_00_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_00_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_00_01/locations.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_00_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_00_01/plaques.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_00_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_00_01/missions.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_00_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_00_01/contacts.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_00_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_00_01/luaBot.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_00_01)

#Atlas Park
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_01_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_01_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_01_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_01_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_01_01/locations.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_01_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_01_01/plaques.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_01_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_01_01/missions.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_01_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_01_01/contacts.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_01_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_01_01/luaBot.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_01_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_01_01/hideAndSeek.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_01_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_01_01/relayRace.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_01_01)

#King's Row
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_01_02/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_01_02)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_01_02/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_01_02)

#Galaxy City
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_01_03/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_01_03)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_01_03/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_01_03)

#Steel Canyon
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_02_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_02_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_02_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_02_01)

#Skyway City
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_02_02/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_02_02)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_02_02/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_02_02)

#Talos Island
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_03_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_03_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_03_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_03_01)

#Independence Port
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_03_02/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_03_02)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_03_02/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_03_02)

#Founders' Falls
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_04_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_04_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_04_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_04_01)

#Brickstown
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_04_02/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_04_02)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_04_02/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_04_02)

#Peregrine Island
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_05_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_05_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/City_05_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/City_05_01)

#Perez Park
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Hazard_01_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Hazard_01_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Hazard_01_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Hazard_01_01)

#Boomtown
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Hazard_02_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Hazard_02_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Hazard_02_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Hazard_02_01)

#Dark Astoria
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Hazard_03_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Hazard_03_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Hazard_03_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Hazard_03_01)

#Crey's Folly
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Hazard_04_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Hazard_04_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Hazard_04_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Hazard_04_01)

#Abandoned Sewer Network
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_01_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_01_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_01_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_01_01)

#Sewer Network
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_01_02/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_01_02)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_01_02/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_01_02)

#Faultline
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_02_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_02_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_02_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_02_01)

#Terra Volta
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_03_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_03_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_03_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_03_01)

#Eden
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_04_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_04_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_04_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_04_01)

#The Hive
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_04_02/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_04_02)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_04_02/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_04_02)

#Rikti Crash Site
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_05_01/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_05_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Trial_05_01/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Trial_05_01)

#Spawndefs
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/CircleOfThorns_Kings.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/CircleOfThorns_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Clockwork_Atlas.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Clockwork_Kings.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Clockwork_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Contaminated_Outbreak.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Hellions_Atlas.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Hellions_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Hydramen_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Police_Robots_Outbreak.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Rikti_Kings.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Rikti_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Shady_Deal_Cop_Outbreak.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Skulls_Kings.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Skulls_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Vahzilok_Atlas.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Vahzilok_Kings.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/scripts/Spawndefs/Vahzilok_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DefaultMapInstances/Spawndefs)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Data/settings_template.cfg ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/settings_template.cfg COPYONLY)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/mysql DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/default_dbs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/pgsql DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/default_dbs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/sqlite DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/default_dbs)
