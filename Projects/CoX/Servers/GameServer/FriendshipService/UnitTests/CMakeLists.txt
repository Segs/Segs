# moc when necessary
set(CMAKE_AUTOMOC ON)

# moc files are generated in the binary dir, look for includes there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_segs_test(FriendshipEventRegistry FriendshipService)
add_segs_test(FriendshipServiceFunctions FriendshipService cox_server_common)

# Retain directory structure in visual studio
get_target_property(TARGET_SOURCES FriendshipEventRegistry SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (FriendshipEventRegistry PROPERTIES
    FOLDER UnitTests/FriendshipService/FriendshipEventRegistry
    RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities/unit_tests
)

get_target_property(TARGET_SOURCES FriendshipServiceFunctions SOURCES) 
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCES}) 
set_target_properties (FriendshipServiceFunctions PROPERTIES
    FOLDER UnitTests/FriendshipService/FriendshipServiceFunctions
    RUNTIME_OUTPUT_DIRECTORY   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utilities/unit_tests
)
