
file(GLOB TEST_SOURCE_LIST *.cpp)
include_directories(${CMAKE_SOURCE_DIR}/third/Catch2/single_include)
add_executable(unittest ${TEST_SOURCE_LIST})
target_link_libraries(unittest aiplayer)

