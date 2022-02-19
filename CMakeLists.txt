cmake_minimum_required(VERSION 3.10)

# set the project name
project(ccex_executor VERSION 1.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(Boost_DEBUG ON)

message("-------------- Checking names -------------")
message("CMAKE_SYSTEM_NAME")
message(${CMAKE_SYSTEM_NAME})
message("ENV{HOME}/software/ccapi/include")
message("$ENV{HOME}/software/ccapi/include")
message("End ENV{HOME}/software/ccapi/include")

message("===========================================")


if(CMAKE_SYSTEM_NAME MATCHES "^MINGW64")
set(CMAKE_C_COMPILER "/mingw64/bin/gcc")
set(CMAKE_CXX_COMPILER "/mingw64/bin/g++")
elseif(CMAKE_SYSTEM_NAME EQUAL Linux)
set(CMAKE_C_COMPILER "/usr/bin/gcc-11")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-11")
endif()

add_compile_options(-v -Wl,--verbose)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_compile_definitions(CCAPI_ENABLE_SERVICE_EXECUTION_MANAGEMENT)
add_compile_definitions(CCAPI_ENABLE_EXCHANGE_BITMEX)
add_compile_definitions(CCAPI_ENABLE_EXCHANGE_FTX)
add_compile_definitions(CCAPI_ENABLE_EXCHANGE_COINBASE)

add_library(ccex_trade STATIC
    CcexOrderExecutor.cpp)

set(SOFTWARE "$ENV{HOME}/software")

target_include_directories(ccex_trade PUBLIC
    "${SOFTWARE}/websocketpp"
    "${SOFTWARE}/ccapi/include"
)

if(CMAKE_SYSTEM_NAME MATCHES "^MINGW64")
  target_link_libraries(ccex_trade 
	  ws2_32)
endif()

target_link_libraries(ccex_trade 
    Boost::json
    Boost::log
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

set(BOOST_ROOT "/usr/local/")
find_package(Boost REQUIRED json)
find_package(Boost REQUIRED log)

find_package(OpenSSL REQUIRED
    SSL
    Crypto)

find_package(Threads REQUIRED)

# add the executable
add_executable(ccex_executor 
    execute_ccex_order.cpp)

target_include_directories(ccex_executor PUBLIC
    "${SOFTWARE}/ccapi/include"
)
    
target_link_libraries(ccex_executor PUBLIC
    ccex_trade
)
