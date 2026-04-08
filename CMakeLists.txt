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

add_executable(hhh
    pure_example/hhh.cpp
)

add_executable(test_compose
    tests/compose.cpp
)
target_link_libraries(test_compose
    GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(test_compose)
