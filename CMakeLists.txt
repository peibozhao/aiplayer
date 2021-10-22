
cmake_minimum_required(VERSION 3.11.0)
project(aiplayer)

option(AIPLAYER_BUILD_DEMO "Build demo or not" OFF)
option(AIPLAYER_BUILD_TOOL "Build test or not" ON)

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
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Yaml build tools or not" FORCE)
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

set(AIPLAYER_SOURCE_LIST
    source/application/blhx_application.cpp
    source/source/minicap_source.cpp
    source/source/image_source.cpp
    source/sink/minitouch_operation.cpp
    source/ocr_detect/paddle_ocr.cpp
    source/player/common_player.cpp
    source/common/util_functions.cpp
    )

add_library(aiplayer SHARED ${AIPLAYER_SOURCE_LIST})
target_link_libraries(aiplayer
  yaml-cpp
  glog
  nlohmann_json
  stdc++fs
  )

if (AIPLAYER_BUILD_DEMO)
    add_subdirectory(demo)
endif()
if (AIPLAYER_BUILD_TOOL)
    add_subdirectory(tool)
endif()
