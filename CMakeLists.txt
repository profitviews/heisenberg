cmake_minimum_required(VERSION 3.10)

# set the project name
project(profitview_socketio_cpp VERSION 1.0)

set(CMAKE_C_COMPILER "/usr/bin/gcc-10")
set(CMAKE_CXX_COMPILER "/bin/g++-10")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(PROJECTS_DIR "$ENV{HOME}/projects")

add_subdirectory(src)

# add the executable
add_executable(profitview_socketio_cpp 
    algo_example.cpp 
    DataStreamer.cpp 
    MarketDataAdapter.cpp
    TalibMeanReversion.cpp)

include_directories(
    "${PROJECTS_DIR}/bitmex_trade_example"
    "/usr/local/include/ta-lib")

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

find_package(Boost REQUIRED log)
find_package(Threads REQUIRED)

target_link_directories(profitview_socketio_cpp PUBLIC
    "${PROJECTS_DIR}/bitmex_trade_example/build")

target_link_libraries(profitview_socketio_cpp PUBLIC 
    SIOClient
    bitmex_trade
    Boost::log
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
    OpenSSL::Crypto
    ta_lib)

target_include_directories(profitview_socketio_cpp PUBLIC 
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/src/include"
    ${PROJECTS_DIR}/bitmex_trade_example)
