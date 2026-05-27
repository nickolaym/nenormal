# test elements
add_test_executable(test_concepts concepts.cpp)
add_test_executable(test_str str.cpp)
add_test_executable(test_str_algo str_algo.cpp)
add_test_executable(test_augmented augmented.cpp)
add_test_executable(test_maybe maybe.cpp)
add_test_executable(test_substitute substitute.cpp)
add_test_executable(test_tristate tristate.cpp)

add_all_above(unittests_elements)

# rules
add_subdirectory(rules)

add_all_above(unittests)
