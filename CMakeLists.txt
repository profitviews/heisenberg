cmake_minimum_required(VERSION 3.27)

# set the project name
project(cpp_crypto_algos LANGUAGES CXX VERSION 2.0)

# compile_commands.json for clangd / VS Code C++ (must exist — IntelliSense uses ${workspaceFolder}/build/compile_commands.json)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json for IDE tooling" FORCE)

# Multi-config generators (Ninja Multi-Config, MSVC): only Debug and Release
if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Available configurations" FORCE)
endif()

# websocketpp (and similar deps) use legacy project() without VERSION; default CMP0048 for subdirs avoids noisy CMake dev warnings.
set(CMAKE_POLICY_DEFAULT_CMP0048 OLD)

include(FetchContent)

FetchContent_Declare(
    ccapi
    GIT_REPOSITORY https://github.com/profitviews/ccapi.git
    GIT_TAG        windows-msvc
)
FetchContent_Declare(
    websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG        develop
)
FetchContent_Declare(
    csv2
    GIT_REPOSITORY https://github.com/p-ranav/csv2
    GIT_TAG        master
)
FetchContent_MakeAvailable(ccapi websocketpp csv2)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake )
find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

configure_file(cmake/cpp_crypto_algos_config.hpp.in cpp_crypto_algos_config.hpp)

include(clang_format)
add_subdirectory(src)