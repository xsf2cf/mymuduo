set(SERVER_LIST server.cc)
set(CLIENT_LIST client.cc)

add_executable(server ${SERVER_LIST})
target_link_libraries(server mymuduo pthread)

add_executable(client ${CLIENT_LIST})
target_link_libraries(client mymuduo pthread)
