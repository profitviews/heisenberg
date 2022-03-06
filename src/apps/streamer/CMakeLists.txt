add_executable(streamer)
target_sources(streamer
    PRIVATE
        abstracted_streamer.cpp
)

target_link_libraries(streamer
    PRIVATE
        cpp_crypto_algos_lib
)