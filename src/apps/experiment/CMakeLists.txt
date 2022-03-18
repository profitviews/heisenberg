add_executable(experiment)
target_sources(experiment
    PRIVATE
        experiment.cpp
)

target_link_libraries(experiment
    PRIVATE
        cpp_crypto_algos_lib
)

set_target_properties(experiment
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)