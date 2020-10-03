cmake_minimum_required(VERSION 3.10)

# set the project name
project(socket_io_cpp_streamer VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)

# add the executable
add_executable(socket_io_cpp_streamer test_pv_markets.cpp DataStreamer.cpp MarketDataAdapter.cpp)

configure_file(socket_io_cpp_streamerConfig.h.in socket_io_cpp_streamerConfig.h)

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

# find_package(Boost REQUIRED ProgramOptions)

target_link_libraries(socket_io_cpp_streamer PUBLIC 
    SIOClient
    # Boost::ProgramOptions
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

target_include_directories(socket_io_cpp_streamer PUBLIC 
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/src/include")
