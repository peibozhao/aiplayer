
set(AIPLAYER_SERVER_SOURCES server/server.cpp server/http_server.cpp)
add_executable(aiplayer_server ${AIPLAYER_SERVER_SOURCES})
target_link_libraries(aiplayer_server
  aiplayer
  )

set(AIPLAYER_CLIENT_SOURCES client/client.cpp client/adb_client.cpp)
add_executable(aiplayer_client ${AIPLAYER_CLIENT_SOURCES})
target_link_libraries(aiplayer_client
  aiplayer
  )
