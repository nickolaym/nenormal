#include "nenormal/str_algo.h"
#include <gtest/gtest.h>

namespace nn {

using namespace ::nn::literals;

TEST(constants, size) {
    constexpr auto n = ct_size_v<1>;
    static_assert(n.value == 1);
    using type = decltype(n);
    static_assert(CtOfType<type, size_t>);
}

TEST(constants, char) {
    constexpr auto c = ct_char_v<'.'>;
    static_assert(c.value == '.');
    using type = decltype(c);
    static_assert(CtOfType<type, char>);
}

TEST(chars, str) {
    static_assert(chars(ct_size_v<0>, '.') == ""_ss);
    static_assert(chars(ct_size_v<1>, '.') == "."_ss);
    static_assert(chars(ct_size_v<5>, '.') == "....."_ss);
}

TEST(chars, ctstr) {
    static_assert(ct_chars(ct_size_v<0>, ct_char_v<'.'>) == ""_cts);
    static_assert(ct_chars(ct_size_v<1>, ct_char_v<'.'>) == "."_cts);
    static_assert(ct_chars(ct_size_v<5>, ct_char_v<'.'>) == "....."_cts);
}

TEST(concat, str) {
    static_assert(concat_str() == ""_ss);
    static_assert(concat_str("abc"_ss) == "abc"_ss);
    static_assert(concat_str("abc"_ss, "def"_ss) == "abcdef"_ss);
    static_assert(concat_str("abc"_ss, "de"_ss, "fghijk"_ss) == "abcdefghijk"_ss);
}

TEST(concat, ctstr) {
    static_assert(concat_ctstr() == ""_cts);
    static_assert(concat_ctstr("abc"_cts) == "abc"_cts);
    static_assert(concat_ctstr("abc"_cts, "def"_cts) == "abcdef"_cts);
    static_assert(concat_ctstr("abc"_cts, "de"_cts, "fghijk"_cts) == "abcdefghijk"_cts);
}

TEST(size_to_str, len) {
    static_assert(size_to_str_len(0) == 1);
    static_assert(size_to_str_len(9) == 1);
    static_assert(size_to_str_len(10) == 2);
    static_assert(size_to_str_len(99) == 2);
    static_assert(size_to_str_len(100) == 3);
    static_assert(size_to_str_len(999) == 3);
    static_assert(size_to_str_len(1000) == 4);
    static_assert(size_to_str_len(9999) == 4);
    static_assert(size_to_str_len(10000) == 5);
}

TEST(size_to_str, str) {
    static_assert(size_to_str(ct_size_v<0>) == "0"_ss);
    static_assert(size_to_str(ct_size_v<9>) == "9"_ss);
    static_assert(size_to_str(ct_size_v<10>) == "10"_ss);
    static_assert(size_to_str(ct_size_v<99>) == "99"_ss);
    static_assert(size_to_str(ct_size_v<100>) == "100"_ss);
    static_assert(size_to_str(ct_size_v<1000>) == "1000"_ss);
    static_assert(size_to_str(ct_size_v<12345>) == "12345"_ss);
}

TEST(size_to_str, ctstr) {
    static_assert(size_to_ctstr(ct_size_v<0>) == "0"_cts);
    static_assert(size_to_ctstr(ct_size_v<9>) == "9"_cts);
    static_assert(size_to_ctstr(ct_size_v<10>) == "10"_cts);
    static_assert(size_to_ctstr(ct_size_v<99>) == "99"_cts);
    static_assert(size_to_ctstr(ct_size_v<100>) == "100"_cts);
    static_assert(size_to_ctstr(ct_size_v<1000>) == "1000"_cts);
    static_assert(size_to_ctstr(ct_size_v<12345>) == "12345"_cts);
}

} // namespace nn
