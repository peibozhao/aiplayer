
cmake_minimum_required(VERSION 3.11.0)
project(aiplayer)

option(AIPLAYER_BUILD_DEMO "Build demo or not" OFF)
option(AIPLAYER_BUILD_TOOL "Build test or not" ON)

include(FetchContent)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)

set(WITH_GFLAGS OFF CACHE BOOL "Glog use gflags" FORCE)
set(BUILD_TESTING OFF)

# glog
FetchContent_Declare(log
    URL https://github.com/google/glog/archive/refs/tags/v0.4.0.tar.gz
    )
FetchContent_MakeAvailable(log)
include_directories(${log_BINARY_DIR})

# yaml
FetchContent_Declare(yaml
    URL https://github.com/jbeder/yaml-cpp/archive/refs/tags/yaml-cpp-0.6.3.tar.gz
    )
FetchContent_MakeAvailable(yaml)
include_directories(${yaml_SOURCE_DIR})

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
    source/image_source/minicap_source.cpp
    source/common/util_functions.cpp
    source/device_operation/minitouch_operation.cpp
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
