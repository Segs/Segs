# moc when necessary
set(CMAKE_AUTOMOC ON)

# moc files are generated in the binary dir, look for includes there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_segs_test(SuperGroupEventRegistry SuperGroupService)
add_segs_test(SuperGroupServiceFunctions SuperGroupService cox_server_common)

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES SuperGroupEventRegistry SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES})
set_target_properties (SuperGroupEventRegistry PROPERTIES
    FOLDER UnitTests/SuperGroupService/SuperGroupEventRegistry
)

get_target_property(TARGET_SOURCES SuperGroupServiceFunctions SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES})
set_target_properties (SuperGroupServiceFunctions PROPERTIES
    FOLDER UnitTests/SuperGroupService/SuperGroupServiceFunctions
)
