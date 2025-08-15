cmake_minimum_required(VERSION 3.10)

project(nenormal)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
find_package(GTest)

# add_executable(nenormal
#     src/static_string.h
#     src/nenormal.cpp
# )

add_executable(test_static_strings
    tests/test_static_strings.cpp
)
target_link_libraries(test_static_strings
    gtest
    gtest_main
)

gtest_discover_tests(test_static_strings)
