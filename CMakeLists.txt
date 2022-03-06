cmake_minimum_required(VERSION 3.22)

# set the project name
project(cpp_crypto_algos LANGUAGES CXX VERSION 2.0)

include(FetchContent)

FetchContent_Declare(
    ccapi
    GIT_REPOSITORY https://github.com/crypto-chassis/ccapi.git
    GIT_TAG        v5.28.3
)
FetchContent_Declare(
    websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG        develop
)
FetchContent_MakeAvailable(ccapi websocketpp)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

configure_file(cmake/cpp_crypto_algos_config.hpp.in cpp_crypto_algos_config.hpp)

add_subdirectory(src)