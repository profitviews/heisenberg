cmake_minimum_required(VERSION 3.10)

# set the project name
project(profitview_socketio_cpp VERSION 1.0)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_COMPILER "/usr/bin/gcc-10")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-10")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(PROJECTS_DIR "$ENV{HOME}/projects")

add_subdirectory(src)

add_library(profitview_socketio STATIC
    MarketDataAdapter.cpp
    profitview_util.cpp)

# add the executable
add_executable(profitview_socketio_cpp
    streaming_example.cpp
    DataStreamer.cpp)

include_directories(
    "${PROJECTS_DIR}/bitmex_executor")

configure_file(profitview_socketio_cppConfig.h.in profitview_socketio_cppConfig.h)

find_package(OpenSSL REQUIRED
    SSL
    Crypto)

find_package(Poco REQUIRED 
    Net 
    JSON 
    Net 
    Util
    Encodings 
    NetSSL 
    Foundation 
    Data 
    Crypto 
    XML)

find_package(Threads REQUIRED)

target_link_directories(profitview_socketio_cpp PUBLIC
    "${PROJECTS_DIR}/bitmex_executor/build")

target_link_libraries(profitview_socketio_cpp PUBLIC 
    profitview_socketio
    SIOClient
    bitmex_trade
    Threads::Threads
    Poco::Net 
    Poco::JSON 
    Poco::Net 
    Poco::Util
    Poco::Encodings 
    Poco::NetSSL 
    Poco::Foundation 
    Poco::Data 
    Poco::Crypto 
    Poco::XML
    OpenSSL::SSL
    OpenSSL::Crypto)

target_include_directories(profitview_socketio_cpp PUBLIC 
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/src/include"
    ${PROJECTS_DIR}/bitmex_executor)
