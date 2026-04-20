#include "nenormal/augmented.h"
#include <gtest/gtest.h>

template<class T> struct foo {
    T t;
    auto rebind(auto u) const { return foo<decltype(u)>{u}; }
};

TEST(augmented, no_augmentation) {
    constexpr auto a = "aaa"_cts;
    constexpr auto b = update_text(a, "rule_goes_here", "bbb"_cts);
    static_assert(b == "bbb"_cts);

    constexpr auto r = rebind_text(b, "rrr"_cts);
    static_assert(r == "rrr"_cts);
}

TEST(augmented, empty) {
    auto a = augmented_text{"aaa"_cts, empty{}};
    auto b = update_text(a, "rule_goes_here", "bbb"_cts);
    static_assert(b.text == "bbb"_cts);
    empty e = b.aux;

    constexpr auto r = rebind_text(b, "rrr"_cts);
    static_assert(r.text == "rrr"_cts);
}

TEST(augmented, side_effect) {
    int counter = 0;
    auto f = [&](auto i, auto p, auto o) {
        std::cout << i << " " << p << " " << o << std::endl;
        ++counter;
    };

    auto a = augmented_text{"aaa"_cts, side_effect{f}};
    EXPECT_EQ(counter, 0);
    auto b = update_text(a, "rule_goes_here", "bbb"_cts);
    static_assert(b.text == "bbb"_cts);
    EXPECT_EQ(counter, 1);
    auto c = update_text(b, "rule_goes_here", "ccc"_cts);
    static_assert(c.text == "ccc"_cts);
    EXPECT_EQ(counter, 2);

    auto r = rebind_text(c, "rrr"_cts);
    static_assert(r.text == "rrr"_cts);
    EXPECT_EQ(counter, 2);  // no side effect
}

TEST(augmented, pure_side_effect) {
    constexpr auto f = [](auto...) {}; // does nothing
    constexpr auto a = augmented_text{"aaa"_cts, side_effect{f}};
    constexpr auto b = update_text(a, "rule_goes_here", "bbb"_cts);
    constexpr auto r = rebind_text(b, "rrr"_cts);
    static_assert(r.text == "rrr"_cts);
}

TEST(augmented, cumulative_effect) {
    auto f = [](auto acc, auto i, auto p, auto o) {
        std::cout << i << " " << p << " " << o << std::endl;
        return acc + 1;
    };

    auto a = augmented_text{"aaa"_cts, cumulative_effect{f, 0}};
    EXPECT_EQ(a.aux.a, 0);
    auto b = update_text(a, "rule_goes_here", "bbb"_cts);
    static_assert(b.text == "bbb"_cts);
    EXPECT_EQ(b.aux.a, 1);
    auto c = update_text(b, "rule_goes_here", "ccc"_cts);
    static_assert(c.text == "ccc"_cts);
    EXPECT_EQ(c.aux.a, 2);

    auto r = rebind_text(c, "rrr"_cts);
    static_assert(r.text == "rrr"_cts);
    EXPECT_EQ(r.aux.a, 2);
}

TEST(augmented, pure_cumulative_effect) {
    constexpr auto f = [](auto acc, auto i, auto p, auto o) {
        return acc + 1;
    };

    constexpr auto a = augmented_text{"aaa"_cts, cumulative_effect{f, 0}};
    EXPECT_EQ(a.aux.a, 0);
    constexpr auto b = update_text(a, "rule_goes_here", "bbb"_cts);
    static_assert(b.text == "bbb"_cts);
    EXPECT_EQ(b.aux.a, 1);
    constexpr auto c = update_text(b, "rule_goes_here", "ccc"_cts);
    static_assert(c.text == "ccc"_cts);
    EXPECT_EQ(c.aux.a, 2);
    static_assert(c.aux.a == 2);

    constexpr auto r = rebind_text(c, "rrr"_cts);
    static_assert(r.text == "rrr"_cts);
    static_assert(r.aux.a == 2);
}
