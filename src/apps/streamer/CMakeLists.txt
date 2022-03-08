add_executable(streamer)
target_sources(streamer
    PRIVATE
        abstracted_streamer.cpp
)

target_link_libraries(streamer
    PRIVATE
        cpp_crypto_algos_lib
)

set_target_properties(streamer
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
