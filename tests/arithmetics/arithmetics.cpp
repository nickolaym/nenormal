#include <gtest/gtest.h>

#include "./program.h"

TEST(arithmetics, one_digit) {
    static_assert(machine(CTSTR("1+1=")) == CTSTR("2"));
    static_assert(machine(CTSTR("2+3=")) == CTSTR("5"));
}
TEST(arithmetics, one_digit_with_carry) {
    static_assert(machine(CTSTR("1+9=")) == CTSTR("10"));
    static_assert(machine(CTSTR("7+8=")) == CTSTR("15"));
}
TEST(arithmetics, short_plus_long) {
    static_assert(machine(CTSTR("1+12345=")) == CTSTR("12346"));
    static_assert(machine(CTSTR("12345+1=")) == CTSTR("12346"));
}
TEST(arithmetics, propagate_carry) {
    static_assert(machine(CTSTR("1+123999=")) == CTSTR("124000")); // carry stops
    static_assert(machine(CTSTR("1+999999=")) == CTSTR("1000000")); // carry exceeds 1 digit
}

TEST(arithmetics, big_nums) {
    static_assert(machine(CTSTR("12345+67890=")) == CTSTR("80235"));
    static_assert(machine(CTSTR("98765+66666=")) == CTSTR("165431"));
}
