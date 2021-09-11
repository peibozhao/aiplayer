
add_executable(fetch_image cpp/fetch_image.cpp)
target_link_libraries(fetch_image aiplayer)

add_executable(click cpp/click.cpp)
target_link_libraries(click aiplayer)
