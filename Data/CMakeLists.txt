file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/motd.smlx DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/global.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Encounter_Manager.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ES_OL_Functions.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Global_NPC_Extras.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Global_Persistents.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/spawners.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Universal_Spawns.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)

set(MAPNAMES
    City_00_01
    City_01_01 City_01_02 City_01_03
    City_02_01 City_02_02
    City_03_01 City_03_02
    City_04_01 City_04_02
    City_05_01
    Hazard_01_01
    Hazard_02_01
    Hazard_03_01
    Hazard_04_01 Hazard_04_02
    Hazard_05_01
    Trial_01_01 Trial_01_02
    Trial_02_01
    Trial_03_01
    Trial_04_01 Trial_04_02
    Trial_05_01
    )

foreach(MAPDIR ${MAPNAMES})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/${MAPDIR}/entities.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/${MAPDIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/${MAPDIR}/locations.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/${MAPDIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/${MAPDIR}/plaques.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/${MAPDIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/${MAPDIR}/missions.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/${MAPDIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/${MAPDIR}/contacts.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/${MAPDIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/${MAPDIR}/ES_Library_Objects.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/${MAPDIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/${MAPDIR}/TimeCop.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/${MAPDIR})
endforeach(MAPDIR)


#Outbreak
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_00_01/luaBot.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_00_01)
#Outbreak Spawndefs
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_00_01/Spawndefs/Contaminated_Outbreak.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_00_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_00_01/Spawndefs/Police_Robots_Outbreak.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_00_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_00_01/Spawndefs/Shady_Deal_Cop_Outbreak.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_00_01/Spawndefs)

#Atlas Park
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_01_01/luaBot.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_01_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_01_01/hideAndSeek.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_01_01)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_01_01/relayRace.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_01_01)
#Atlas Park Spawndefs
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_01_01/Spawndefs/Clockwork_Atlas.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_01_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_01_01/Spawndefs/Hellions_Atlas.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_01_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_01_01/Spawndefs/Vahzilok_Atlas.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_01_01/Spawndefs)

#King's Row Spawndefs
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_01_02/Spawndefs/CircleOfThorns_Kings.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_01_02/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_01_02/Spawndefs/Clockwork_Kings.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_01_02/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_01_02/Spawndefs/Rikti_Kings.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_01_02/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_01_02/Spawndefs/Skulls_Kings.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_01_02/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_01_02/Spawndefs/Vahzilok_Kings.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_01_02/Spawndefs)

#Steel Canyon Spawndefs
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_02_01/Spawndefs/5th_Steel.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_02_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_02_01/Spawndefs/CircleOfThorns_Steel.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_02_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_02_01/Spawndefs/Clockwork_Steel.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_02_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_02_01/Spawndefs/Outcasts_Steel.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_02_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_02_01/Spawndefs/Tsoo_Steel.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_02_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_02_01/Spawndefs/Vahzilok_Steel.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_02_01/Spawndefs)

#Skyway City Spawndefs
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_02_02/Spawndefs/Clockwork_Skyway.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_02_02/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_02_02/Spawndefs/Rikti_Skyway.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_02_02/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_02_02/Spawndefs/Trolls_Skyway.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_02_02/Spawndefs)

#Talos Island Spawndefs
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_03_01/Spawndefs/Banished_Talos.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_03_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_03_01/Spawndefs/CircleOfThorns_Talos.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_03_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_03_01/Spawndefs/Devouring_Talos.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_03_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_03_01/Spawndefs/Freakshow_Talos.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_03_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_03_01/Spawndefs/Tsoo_Talos.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_03_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_03_01/Spawndefs/Warriors_Talos.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_03_01/Spawndefs)

#Independence Port Spawndefs
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_03_02/Spawndefs/5th_IndPort.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_03_02/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_03_02/Spawndefs/Devouring_IndPort.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_03_02/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_03_02/Spawndefs/Family_IndPort.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_03_02/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/City_03_02/Spawndefs/Tsoo_IndPort.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/City_03_02/Spawndefs)

#Perez Park Spawndefs
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_01_01/Spawndefs/CircleOfThorns_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_01_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_01_01/Spawndefs/Clockwork_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_01_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_01_01/Spawndefs/Hellions_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_01_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_01_01/Spawndefs/Hydramen_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_01_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_01_01/Spawndefs/Rikti_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_01_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_01_01/Spawndefs/Skulls_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_01_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_01_01/Spawndefs/Vahzilok_Perez.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_01_01/Spawndefs)

#Boomtown Spawndefs
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_02_01/Spawndefs/5th_Boomtown.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_02_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_02_01/Spawndefs/Clockwork_Boomtown.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_02_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_02_01/Spawndefs/Outcasts_Boomtown.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_02_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_02_01/Spawndefs/Rikti_Boomtown.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_02_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_02_01/Spawndefs/Trolls_Boomtown.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_02_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_02_01/Spawndefs/Vahzilok_Boomtown.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_02_01/Spawndefs)

#Dark Astoria Spawndefs
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_03_01/Spawndefs/Banished_DarkAstoria.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_03_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_03_01/Spawndefs/CircleOfThorns_DarkAstoria.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_03_01/Spawndefs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/Hazard_03_01/Spawndefs/Tsoo_DarkAstoria.spawndef.lua DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/Hazard_03_01/Spawndefs)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/settings_template.cfg ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/default_setup/settings_template.cfg COPYONLY)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/mysql DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/default_setup)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pgsql DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/default_setup)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sqlite DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/default_setup)


file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data)

set(MAPNAMES
  City_00_01
  City_01_01 City_01_02 City_01_03
  City_02_01 City_02_02
  City_03_01 City_03_02
  City_04_01 City_04_02
  City_05_01
  Hazard_01_01
  Hazard_02_01
  Hazard_03_01
  Hazard_04_01 Hazard_04_02
  Hazard_05_01
  Trial_01_01 Trial_01_02
  Trial_02_01
  Trial_03_01
  Trial_04_01 Trial_04_02
  Trial_05_01
  Mission_5th_Column
  Mission_Abandoned_Office
  Mission_Abandoned_Warehouse
  Mission_Caves
  Mission_COT
  Mission_Office
  Mission_Outdoor_City
  Mission_Outdoor_Forest
  Mission_Outdoor_Industrial
  Mission_Outdoor_Missions
  Mission_Outdoor_Ruined
  Mission_Sewers
  Mission_Tech
  Mission_unique
  Mission_Warehouse
  )
foreach(MAPDIR ${MAPNAMES})
  file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/maps/${MAPDIR})
endforeach(MAPDIR)
