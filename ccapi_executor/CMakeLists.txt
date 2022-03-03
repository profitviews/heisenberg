add_library(ccapi_executor CcexOrderExecutor.cpp)
set(SOFTWARE "$ENV{HOME}/software")
target_include_directories(ccapi_executor PUBLIC
    "${SOFTWARE}/websocketpp"
    "${SOFTWARE}/ccapi/include"
)
