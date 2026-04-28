cmake_minimum_required(VERSION 3.10)

project(nenormal)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the compilation flag for concepts diagnostics depth
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts-diagnostics-depth=10")


enable_testing()
find_package(GTest)

#####

# Interface library for global includes
add_library(nenormal_headers INTERFACE)
target_include_directories(nenormal_headers INTERFACE ${CMAKE_SOURCE_DIR}/include)

# Link to all executables automatically (must be before add_executable)
link_libraries(nenormal_headers)

# Function to create test executable with gtest_discover_tests
function(add_test_executable name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(${name})
endfunction()

# test elements
add_test_executable(test_str tests/str.cpp)
add_test_executable(test_augmented tests/augmented.cpp)
add_test_executable(test_compose tests/compose.cpp)
add_test_executable(test_rules tests/rules.cpp)
add_test_executable(test_maybe tests/maybe.cpp)

# test examples
add_test_executable(test_hello_world examples/hello_world.cpp)
add_test_executable(test_correct_bracket_sequence examples/correct_bracket_sequence.cpp)

add_test_executable(test_arithmetics examples/arithmetics/arithmetics.cpp)
add_test_executable(test_arithmetics_runtime examples/arithmetics/arithmetics_runtime.cpp)

add_test_executable(test_collatz examples/collatz/collatz.cpp)
add_test_executable(test_collatz_decimal examples/collatz_decimal/collatz_decimal.cpp)

add_test_executable(test_monadic experimental/monadic.cpp)
