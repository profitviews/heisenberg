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
find_package(Boost REQUIRED log json)
find_package(OpenSSL REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

configure_file(cpp_crypto_algosConfig.h.in cpp_crypto_algosConfig.h)

# add the executable
add_executable(cpp_crypto_algos "")
target_sources(cpp_crypto_algos
    PRIVATE
        abstracted_algo.cpp
)

target_link_libraries(cpp_crypto_algos
    PRIVATE 
        Boost::log
        Boost::json
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
        RapidJSON::RapidJSON
)

target_include_directories(cpp_crypto_algos
    PRIVATE 
        ${PROJECT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/ccapi_executor
        ${ccapi_SOURCE_DIR}/include
        ${websocketpp_SOURCE_DIR}
)

target_compile_definitions(cpp_crypto_algos
    PRIVATE
        CCAPI_ENABLE_SERVICE_EXECUTION_MANAGEMENT
        CCAPI_ENABLE_SERVICE_MARKET_DATA
        CCAPI_ENABLE_EXCHANGE_BITMEX
        CCAPI_ENABLE_EXCHANGE_FTX
        CCAPI_ENABLE_EXCHANGE_COINBASE
)

target_compile_options(cpp_crypto_algos
    PRIVATE
        $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wno-terminate>
)