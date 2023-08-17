cmake_minimum_required(VERSION 3.22)

project(asmsd VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_EXAMPLE OFF CACHE BOOL "do not build inotify example")
set(BUILD_TEST OFF CACHE BOOL "do not build inotify test")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "do not build inotify as shared lib")
set(BUILD_STATIC_LIBS ON CACHE BOOL "build inotify as static lib only")
set(HTTPLIB_INSTALL OFF CACHE BOOL "use httplib as header only")
set(CXXOPTS_BUILD_EXAMPLES OFF CACHE BOOL "do not build CXXOPTS examples")
set(CXXOPTS_ENABLE_INSTALL OFF CACHE BOOL "use CXXOPTS as header only")

find_package(fmt REQUIRED)

include(FetchContent)

FetchContent_Declare(cxxopts 
    URL https://github.com/jarro2783/cxxopts/archive/refs/tags/v3.1.1.tar.gz)
FetchContent_MakeAvailable(cxxopts)

FetchContent_Declare(json 
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(httplib
    URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.12.6.tar.gz)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(json-rpc-cxx
    URL https://github.com/jsonrpcx/json-rpc-cxx/archive/refs/tags/v0.3.1.tar.gz)
FetchContent_MakeAvailable(json-rpc-cxx)

FetchContent_Declare(spdlog
    URL https://github.com/gabime/spdlog/archive/refs/tags/v1.11.0.tar.gz)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(inotify-cpp
    URL https://github.com/erikzenker/inotify-cpp/archive/refs/tags/v1.0.0.tar.gz)
FetchContent_MakeAvailable(inotify-cpp)

execute_process(
    COMMAND patch ${httplib_SOURCE_DIR}/httplib.h ${PROJECT_SOURCE_DIR}/httplib.h.diff
)

add_executable(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${cxxopts_SOURCE_DIR}/include
    #${json_SOURCE_DIR}/include
    ${httplib_SOURCE_DIR}
    ${json-rpc-cxx_SOURCE_DIR}/include
    ${spdlog_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/include
)
target_sources(${PROJECT_NAME} 
    PRIVATE 
    src/main.cpp 
    src/messages.cpp
    src/device.cpp
    src/httpclientconnector.cpp
)
target_link_libraries(${PROJECT_NAME}
    PRIVATE 
    fmt::fmt
    inotify-cpp::inotify-cpp
    nlohmann_json::nlohmann_json
)
target_compile_options(${PROJECT_NAME}
PRIVATE -Wall -Wextra -Wpedantic -Werror
)