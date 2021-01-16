cmake_minimum_required(VERSION 3.10)

# set the project name
project(cpp_crypto_algos VERSION 1.0)

set(CMAKE_C_COMPILER "/usr/bin/gcc-10")
set(CMAKE_CXX_COMPILER "/bin/g++-10")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(PROJECTS_DIR "$ENV{HOME}/projects")

# add the executable
add_executable(cpp_crypto_algos
    algo_example.cpp
    TalibMeanReversion.cpp)

configure_file(cpp_crypto_algosConfig.h.in cpp_crypto_algosConfig.h)

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

target_link_directories(cpp_crypto_algos PUBLIC
    "${PROJECTS_DIR}/bitmex_trade_example/build"
    "${PROJECTS_DIR}/profitview_socketio_cpp/build"
    "${PROJECTS_DIR}/profitview_socketio_cpp/build/src")

target_link_libraries(cpp_crypto_algos PUBLIC 
    profitview_socketio
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

target_include_directories(cpp_crypto_algos PUBLIC 
    "${PROJECT_BINARY_DIR}"
    "/usr/local/include/ta-lib"
    "${PROJECTS_DIR}/profitview_socketio_cpp"
    "${PROJECTS_DIR}/profitview_socketio_cpp/src/include"
    ${PROJECTS_DIR}/bitmex_trade_example)
