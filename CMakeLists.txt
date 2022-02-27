cmake_minimum_required(VERSION 3.10)

# set the project name
project(profitview_socketio_cpp VERSION 1.0)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_COMPILER "/usr/bin/gcc-11")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-11")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)

add_library(profitview_socketio STATIC
    profitview_util.cpp)

# PROJECT_BINARY_DIR is the root of where CMake will write the build output
# PROJECT_SOURCE_DIR is the root of where CMake will take its input files
# These are stated in the `cmake` command, e.g.
# cmake ..
# sets PROJECT_SOURCE_DIR to be the parent directory and PROJECT_BINARY_DIR
# to be the current directory
target_include_directories(profitview_socketio PUBLIC 
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/src/include")

# add the executable
add_executable(profitview_socketio_cpp
    abstracted_streamer.cpp)

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

target_link_libraries(profitview_socketio_cpp PUBLIC 
    profitview_socketio
    SIOClient
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

