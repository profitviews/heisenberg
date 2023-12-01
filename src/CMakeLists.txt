add_library(cpp_crypto_algos_lib
    INTERFACE
        cc_simple_mr.hpp
        cc_kaufman.hpp
        cc_damped_mr.hpp
        cc_trade_handler.hpp
        cc_trade_stream.hpp
        ccex_order_executor.hpp
        enum.hpp
        exchange.hpp
        order_executor.hpp
        order_type.hpp
        program_options.hpp
        side.hpp
        trade_data.hpp
        trade_stream.hpp
        trade_stream_exception.hpp
        trade_stream_maker.hpp
        utils.hpp
        wscc_trade_stream.hpp
)

target_link_libraries(cpp_crypto_algos_lib
    INTERFACE
        Boost::headers
        Boost::log
        Boost::program_options
        fmt::fmt
        OpenSSL::SSL
        OpenSSL::Crypto
        rapidjson
        Threads::Threads
)

target_include_directories(cpp_crypto_algos_lib
    INTERFACE 
        ${PROJECT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/ccapi_executor
        ${ccapi_SOURCE_DIR}/include
        ${websocketpp_SOURCE_DIR}
        ${csv2_SOURCE_DIR}/include
)

target_compile_definitions(cpp_crypto_algos_lib
    INTERFACE
        CCAPI_ENABLE_SERVICE_EXECUTION_MANAGEMENT
        CCAPI_ENABLE_SERVICE_MARKET_DATA
        CCAPI_ENABLE_EXCHANGE_BITMEX
        CCAPI_ENABLE_EXCHANGE_FTX
        CCAPI_ENABLE_EXCHANGE_COINBASE
)

target_compile_options(cpp_crypto_algos_lib
    INTERFACE
        $<$<CXX_COMPILER_ID:GNU>:-Wno-terminate>
        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)
set_property(TARGET MY_LIB PROPERTY CPP_VISIBILITY_PRESET  ON)
set_property(TARGET MY_LIB PROPERTY VISIBILITY_INLINES_HIDDEN ON)

clang_format_target(TARGET cpp_crypto_algos_lib)
add_subdirectory(apps)