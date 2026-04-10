cmake_minimum_required(VERSION 3.10)

project(nenormal)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

add_test_executable(test_compose tests/compose.cpp)
add_test_executable(test_hello_world tests/hello_world.cpp)
add_test_executable(test_correct_bracket_sequence tests/correct_bracket_sequence.cpp)
add_test_executable(test_arithmetics tests/arithmetics.cpp)
