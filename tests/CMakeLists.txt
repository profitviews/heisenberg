include(FetchContent)

# FetchContent exposes PascalCase commands; calling them via cmake_language(CALL ...) keeps this file
# aligned with lowercase-command style checks while remaining valid on CMake 3.27+.
cmake_language(CALL FetchContent_Declare Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
)
cmake_language(CALL FetchContent_MakeAvailable Catch2)

add_executable(heisenberg_tests
    test_utils.cpp
    test_enum.cpp
    test_e2e_sandbox.cpp
    test_exchange_names.cpp
    test_order_completion.cpp
)

target_link_libraries(heisenberg_tests
    PRIVATE
        Catch2::Catch2WithMain
        cpp_crypto_algos_lib
)

target_include_directories(heisenberg_tests PRIVATE
    "${Catch2_SOURCE_DIR}/src"
    "${Catch2_BINARY_DIR}/generated-includes"
)

if(MSVC)
    target_compile_options(heisenberg_tests PRIVATE /bigobj)
endif()

add_test(NAME Heisenberg.Unit COMMAND $<TARGET_FILE:heisenberg_tests>)
