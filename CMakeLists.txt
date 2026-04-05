cmake_minimum_required(VERSION 3.10)

project(nenormal)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
find_package(GTest)

#####

add_executable(hhh
    pure_example/hhh.cpp
)
