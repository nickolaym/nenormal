add_subdirectory(arithmetics)

add_subdirectory(collatz)
add_subdirectory(collatz_decimal)
add_subdirectory(collatz_mini)

add_custom_target(examples_collatz_all
    DEPENDS
        examples_collatz
        examples_collatz_decimal
        examples_collatz_mini
)

# simple examples
add_test_executable(example_hello_world hello_world.cpp)
add_test_executable(example_correct_bracket_sequence correct_bracket_sequence.cpp)

add_custom_target(examples_simple
    DEPENDS
        example_hello_world
        example_correct_bracket_sequence
)

add_all_above(examples)
