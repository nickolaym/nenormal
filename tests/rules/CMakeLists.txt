# test rules
add_test_executable(test_rules rules.cpp)
add_test_executable(test_rule_loop rule_loop.cpp)
add_test_executable(test_augmentation_ctors augmentation_ctors.cpp)

add_all_above(unittests_rules)
