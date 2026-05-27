cmake_minimum_required(VERSION 3.10)
project(nenormal)

include(cmake/settings.cmake)
include(cmake/utilities.cmake)

# Interface library for global includes
add_library(nenormal_headers INTERFACE)
target_include_directories(nenormal_headers INTERFACE ${CMAKE_SOURCE_DIR}/include)

# Link to all executables automatically (must be before add_executable)
link_libraries(nenormal_headers)

# Source directories
add_subdirectory(tests)
add_subdirectory(examples)
add_subdirectory(experimental)
