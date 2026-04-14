#include "nenormal/str.h"
#include <gtest/gtest.h>
#include "./utils.h"

TEST(str, compile_time_properties) {
    constexpr Str auto s = str{"abc"};
    Str auto const& ref = s;
    using s_type = std::remove_const_t<decltype(s)>;
    static_assert(std::same_as<s_type, str<4>>);
    static_assert(Str<s_type>);
    static_assert(s.size() == 3);
    static_assert(s == str{"abc"});
    static_assert(s != str{"def"});
    static_assert(s != str{"a"});
    static_assert(s[0] == 'a');
    static_assert(s[3] == 0);
    static_assert(s.view() == "abc");
}

TEST(str, empty) {
    constexpr auto s = str{""};
    EXPECT_EQ(s.size(), 0u);
    EXPECT_EQ(s.begin(), s.end());
    EXPECT_EQ(*s.begin(), 0); // null-terminated
    EXPECT_EQ(s[0], 0);
    EXPECT_EQ(std::string_view(s), "");
}

TEST(str, single_char) {
    auto s = str{"a"};
    EXPECT_EQ(s.size(), 1u);
    EXPECT_EQ(std::distance(s.begin(), s.end()), 1);
    EXPECT_EQ(*s.begin(), 'a');
    EXPECT_EQ(*s.end(), 0); // null-terminated
    EXPECT_EQ(std::string_view(s), "a");
    EXPECT_EQ(s[0], 'a');
    EXPECT_EQ(s[1], 0);
    // modify
    s[0] = 'b';
    EXPECT_EQ(std::string_view(s), "b");
    EXPECT_EQ(s[0], 'b');
    EXPECT_EQ(s[1], 0);
    // reassign (of same size)
    s = str{"c"};
    EXPECT_EQ(std::string_view(s), "c");
    EXPECT_EQ(s[0], 'c');
    EXPECT_EQ(s[1], 0);
}

TEST(str, multi_char) {
    auto s = str{"hello"};
    EXPECT_EQ(s.size(), 5u);
    EXPECT_EQ(std::distance(s.begin(), s.end()), 5);
    EXPECT_EQ(std::string_view(s), "hello");
    EXPECT_EQ(s[0], 'h');
    EXPECT_EQ(s[4], 'o');
    EXPECT_EQ(s[5], 0);
    EXPECT_EQ(s, str{"hello"});
    EXPECT_NE(s, str{"world"});
    EXPECT_NE(s, str{"h"});
    EXPECT_NE(s, str{"hello!"});
    // modify
    s[4] = '!';
    EXPECT_EQ(s.view(), "hell!");
    EXPECT_EQ(s, str{"hell!"});
    // reassign (same size)
    s = str{"world"};
    EXPECT_EQ(s.view(), "world");
    // fail to reassign (different size)
    static_assert(WILL_NOT_RESOLVE(s = str{"h"}));
}


TEST(str, literal) {
    static_assert(""_ss == str{""});
    static_assert("abc"_ss == str{"abc"});
}

TEST(ct_str, compile_time) {
    constexpr auto s = str{"abc"};
    constexpr auto cts = ct<s>{};
    using s_type = std::remove_const_t<decltype(s)>;
    using cts_type = std::remove_const_t<decltype(cts)>;
    static_assert(std::same_as<cts_type::type, s_type>);
    static_assert(Ct<cts_type>);
    static_assert(CtStr<cts_type>);
    static_assert(cts.value == s);
    static_assert(cts == ct<str{"abc"}>{});
    static_assert(cts != ct<str{"def"}>{});
    static_assert(cts != ct<str{"a"}>{});
}

TEST(ct_str, literal) {
    static_assert("abc"_cts == ct<str{"abc"}>{});
}