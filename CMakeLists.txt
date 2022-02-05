
cmake_minimum_required(VERSION 3.11.0)
project(aiplayer)

option(AIPLAYER_BUILD_DEMO "Build demo or not" OFF)

include(FetchContent)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)

set(BUILD_TESTING OFF CACHE BOOL "Build unitest or not" FORCE)

# glog
FetchContent_Declare(log
    URL https://github.com/google/glog/archive/refs/tags/v0.4.0.tar.gz
    )
FetchContent_MakeAvailable(log)
set(WITH_GFLAGS OFF CACHE BOOL "Glog use gflags or not" FORCE)
include_directories(${log_BINARY_DIR})
target_compile_options(glog PRIVATE -w)

# yaml
FetchContent_Declare(yaml
    URL https://github.com/jbeder/yaml-cpp/archive/refs/tags/yaml-cpp-0.6.3.tar.gz
    )
FetchContent_MakeAvailable(yaml)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Yaml build test or not" FORCE)
include_directories(${yaml_SOURCE_DIR})
target_compile_options(yaml-cpp PRIVATE -w)

# json
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.9.1.tar.gz
    )
set(JSON_BuildTests CACHE STRING OFF)
FetchContent_MakeAvailable(json)
include_directories(${json_SOURCE_DIR}/include)

# http
FetchContent_Declare(http
    URL https://github.com.cnpmjs.org/yhirose/cpp-httplib/archive/refs/tags/v0.8.8.tar.gz
    )
FetchContent_MakeAvailable(http)
include_directories(${http_SOURCE_DIR})

include_directories(source)
include_directories(opencv4)

set(AIPLAYER_SOURCE_LIST
    source/application/common_application.cpp
    source/application/parser/yaml_parser.cpp
    source/input/image/scrcpy_input.cpp
    source/input/image/minicap_input.cpp
    source/input/image/file_input.cpp
    source/input/request/http_request.cpp
    source/output/operation/minitouch_operation.cpp
    source/output/operation/scrcpy_operation.cpp
    source/output/notify/miao_notify.cpp
    source/ocr/paddle_ocr.cpp
    source/player/common_player.cpp
    source/utils/util_functions.cpp
    source/utils/image_utils.cpp
    )

add_library(aiplayer SHARED ${AIPLAYER_SOURCE_LIST})
target_link_libraries(aiplayer
  yaml-cpp
  glog
  nlohmann_json
  stdc++fs
  opencv_core
  opencv_imgcodecs
  )

if (AIPLAYER_BUILD_DEMO)
    add_subdirectory(demo)
endif()
