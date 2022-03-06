add_executable(algo)
target_sources(algo
    PRIVATE
        abstracted_algo.cpp
)

target_link_libraries(algo
    PRIVATE
        cpp_crypto_algos_lib
)