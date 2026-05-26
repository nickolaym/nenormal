cmake_minimum_required(VERSION 3.10)

project(nenormal)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the compilation flag for concepts diagnostics depth
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts-diagnostics-depth=10")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconstexpr-depth=500") # default=512..900


enable_testing()
find_package(GTest)

#####

# Interface library for global includes
add_library(nenormal_headers INTERFACE)
target_include_directories(nenormal_headers INTERFACE ${CMAKE_SOURCE_DIR}/include)

# Link to all executables automatically (must be before add_executable)
link_libraries(nenormal_headers)

# Function to create test executable with gtest_discover_tests
function(add_test_executable name)
    set(sources ${ARGN})
    add_executable(${name} ${sources})
    target_link_libraries(${name} GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(${name})
endfunction()

# Macro to collect all targets above in the file
macro(add_all_above name)
    get_property(_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    get_property(_subdirs DIRECTORY PROPERTY SUBDIRECTORIES)
    foreach(_subdir IN LISTS _subdirs)
        get_property(_subdir_targets DIRECTORY "${_subdir}" PROPERTY BUILDSYSTEM_TARGETS)
        list(APPEND _targets ${_subdir_targets})
    endforeach()
    add_custom_target(${name} DEPENDS ${_targets})
endmacro()

# Source directories
add_subdirectory(tests)
add_subdirectory(examples)
add_subdirectory(experimental)
