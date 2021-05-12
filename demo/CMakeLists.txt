
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.9.1.tar.gz
    )
set(JSON_BuildTests CACHE STRING OFF)
FetchContent_MakeAvailable(json)

FetchContent_Declare(http
    URL https://github.com.cnpmjs.org/yhirose/cpp-httplib/archive/refs/tags/v0.8.8.tar.gz
    )
FetchContent_MakeAvailable(http)

include_directories(${http_SOURCE_DIR})

set(AIPLAYER_SERVER_SOURCES server/server.cpp server/http_server.cpp)
add_executable(aiplayer_server ${AIPLAYER_SERVER_SOURCES})
target_link_libraries(aiplayer_server
  aiplayer
  )

set(AIPLAYER_CLIENT_SOURCES adb_client/client.cpp)
add_executable(aiplayer_client ${AIPLAYER_CLIENT_SOURCES})
target_link_libraries(aiplayer_client
  aiplayer
  )
