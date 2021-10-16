

# include_directories(${http_SOURCE_DIR})

set(AIPLAYER_DEMO_SOURCE_LIST demo.cpp)
add_executable(aipalyer_demo ${AIPLAYER_DEMO_SOURCE_LIST})
target_link_libraries(aipalyer_demo aiplayer)
