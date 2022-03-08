add_executable(algo)
target_sources(algo
    PRIVATE
        abstracted_algo.cpp
)

target_link_libraries(algo
    PRIVATE
        cpp_crypto_algos_lib
)

set_target_properties(algo
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
