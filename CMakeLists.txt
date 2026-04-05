cmake_minimum_required(VERSION 3.10)

project(nenormal)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
find_package(GTest)

#####

add_library(libnenormal
    INTERFACE
    src/size.h
    src/static_string.h
    src/value_type.h
)


#####

add_executable(nenormal
    src/static_string.h
    src/nenormal.cpp
)
target_link_libraries(nenormal
    libnenormal
)

#####

add_executable(test_static_strings
    tests/test_static_strings.cpp
)
target_link_libraries(test_static_strings
    libnenormal
    gtest
    gtest_main
)

gtest_discover_tests(test_static_strings)

######

add_executable(test_func_compose
    tests/test_func_compose.cpp
)
target_link_libraries(test_func_compose
    libnenormal
    gtest
    gtest_main
)

gtest_discover_tests(test_func_compose)

######

add_executable(test_func_loop
    tests/test_func_loop.cpp
)
target_link_libraries(test_func_loop
    libnenormal
    gtest
    gtest_main
)

gtest_discover_tests(test_func_loop)
