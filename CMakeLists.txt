
cmake_minimum_required(VERSION 3.10)
project(aiplayer)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(SPDLOG_FMT_EXTERNAL ON)
set(BUILD_TESTING OFF)
set(MNN_BUILD_TOOLS OFF)

include_directories(src)
include_directories(third/yaml-cpp/include)
include_directories(third/spdlog/include)
include_directories(third/fmt/include)
include_directories(/usr/local/include/opencv4)

file(GLOB_RECURSE SOURCE_LIST
  src/*.cc
  )

add_library(aiplayer SHARED ${SOURCE_LIST})
target_link_libraries(aiplayer
  MNN
  yaml-cpp
  fmt
  spdlog
  opencv_imgcodecs
  opencv_imgproc
  opencv_core
  opencv_videoio
  )

add_subdirectory(third/yaml-cpp)
add_subdirectory(third/fmt)
add_subdirectory(third/spdlog)
add_subdirectory(third/MNN)

add_custom_target(MNN_PRE
  COMMAND ./schema/generate.sh
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/third/MNN
  )

add_dependencies(MNN MNN_PRE)

add_subdirectory(unittest)
add_subdirectory(demo)
