cmake_minimum_required(VERSION 3.10)

# set the project name
project(cpp_crypto_algos VERSION 2.0)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_COMPILER "/usr/bin/gcc-11")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-11")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(CCAPI_ENABLE_SERVICE_EXECUTION_MANAGEMENT)
add_compile_definitions(CCAPI_ENABLE_SERVICE_MARKET_DATA)
add_compile_definitions(CCAPI_ENABLE_EXCHANGE_BITMEX)
add_compile_definitions(CCAPI_ENABLE_EXCHANGE_FTX)
add_compile_definitions(CCAPI_ENABLE_EXCHANGE_COINBASE)

add_subdirectory(ccapi_executor)

set(SOFTWARE "$ENV{HOME}/software")

configure_file(cpp_crypto_algosConfig.h.in cpp_crypto_algosConfig.h)

# add the executable
add_executable(cpp_crypto_algos
    abstracted_algo.cpp)

find_package(OpenSSL REQUIRED
    SSL
    Crypto)

find_package(Boost REQUIRED 
    log
    json)
find_package(Threads REQUIRED)

target_link_libraries(cpp_crypto_algos PUBLIC 
    ccapi_executor
    Boost::log
    Boost::json
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_include_directories(cpp_crypto_algos PUBLIC 
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/trade_stream/include"
    "${PROJECT_SOURCE_DIR}/ccapi_executor"
    "${SOFTWARE}/ccapi/include"
    "${SOFTWARE}/websocketpp"
)
