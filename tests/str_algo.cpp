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

} // namespace nn
