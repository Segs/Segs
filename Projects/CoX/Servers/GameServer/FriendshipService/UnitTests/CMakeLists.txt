# moc when necessary
set(CMAKE_AUTOMOC ON)

# moc files are generated in the binary dir, look for includes there
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_segs_test(FriendshipEventRegistry friendshipservice_lib)
add_segs_test(FriendshipServiceFunctions friendshipservice_lib cox_server_common)
