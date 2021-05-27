
FetchContent_Declare(catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v2.13.6.tar.gz
    )
FetchContent_MakeAvailable(catch2)

file(GLOB TEST_SOURCE_LIST *.cpp)
include_directories(${catch2_SOURCE_DIR}/single_include)
add_executable(unittest ${TEST_SOURCE_LIST})
target_link_libraries(unittest aiplayer)

