# Function to create test executable with gtest_discover_tests
function(add_test_executable name)
    set(sources ${ARGN})
    add_executable(${name} ${sources})
    target_link_libraries(${name}
        nenormal_headers
        GTest::gtest_main
    )
    include(GoogleTest)
    gtest_discover_tests(${name})
endfunction()

function(add_test_executables_here prefix)
    file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    foreach(SOURCE_FILE ${SOURCES})
        get_filename_component(FILE_NAME ${SOURCE_FILE} NAME_WE)
        set(TARGET_NAME "${prefix}${FILE_NAME}")
        add_test_executable(${TARGET_NAME} ${SOURCE_FILE})
        message(STATUS "Created executable target: ${TARGET_NAME} from ${SOURCE_FILE}")
    endforeach()
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
