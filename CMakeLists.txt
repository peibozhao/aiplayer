
cmake_minimum_required(VERSION 3.10)
project(aiplayer)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(SPDLOG_FMT_EXTERNAL ON)
set(BUILD_TESTING OFF)
set(MNN_BUILD_TOOLS OFF)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
endif()

include_directories(src)
include_directories(third/yaml-cpp/include)
include_directories(third/spdlog/include)
include_directories(third/fmt/include)
include_directories(third/cpp-httplib)
include_directories(third/json/include)
include_directories(/usr/local/include/opencv4)

file(GLOB_RECURSE SOURCE_LIST
  src/*.cpp
  )

add_library(aiplayer SHARED ${SOURCE_LIST})
target_link_libraries(aiplayer
  MNN
  yaml-cpp
  fmt
  spdlog
  nlohmann_json
  opencv_imgcodecs
  opencv_imgproc
  opencv_core
  opencv_videoio
  )

add_subdirectory(third/yaml-cpp)
add_subdirectory(third/fmt)
add_subdirectory(third/spdlog)
add_subdirectory(third/MNN)
add_subdirectory(third/json)

add_custom_target(MNN_PRE
  COMMAND ./schema/generate.sh
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/third/MNN
  )

foreach(MNN_TARGET ${MNN_TARGETS})
  add_dependencies(${MNN_TARGET} MNN_PRE)
endforeach()

add_subdirectory(unittest)
add_subdirectory(demo)
