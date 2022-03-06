add_executable(execute_order)
target_sources(execute_order
    PRIVATE
        execute_ccex_order.cpp
)

target_link_libraries(execute_order
    PRIVATE
        cpp_crypto_algos_lib
)