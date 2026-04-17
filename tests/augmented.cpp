#include "nenormal/augmented.h"
#include <gtest/gtest.h>

template<class T> struct foo {
    T t;
    auto rebind(auto u) const { return foo<decltype(u)>{u}; }
};

TEST(augmented, no_augmentation) {
    constexpr auto a = "aaa"_cts;
    constexpr auto b = update_text(a, "s"_cts, "r"_cts, "bbb"_cts);
    static_assert(b == "bbb"_cts);
}

TEST(augmented, empty) {
    auto a = augmented_text{"aaa"_cts, empty{}};
    auto b = update_text(a, "s"_cts, "r"_cts, "bbb"_cts);
    static_assert(b.text == "bbb"_cts);
    empty e = b.aux;
}

TEST(augmented, side_effect) {
    int counter = 0;
    auto f = [&](auto i, auto s, auto r, auto o) {
        std::cout << i << " " << s << " " << r << " " << o << std::endl;
        ++counter;
    };

    auto a = augmented_text{"aaa"_cts, side_effect{f}};
    EXPECT_EQ(counter, 0);
    auto b = update_text(a, "s"_cts, "r"_cts, "bbb"_cts);
    static_assert(b.text == "bbb"_cts);
    EXPECT_EQ(counter, 1);
    auto c = b("s"_cts, "r"_cts, "ccc"_cts);
    static_assert(c.text == "ccc"_cts);
    EXPECT_EQ(counter, 2);
}

TEST(augmented, pure_side_effect) {
    constexpr auto f = [](auto...) {}; // does nothing
    constexpr auto a = augmented_text{"aaa"_cts, side_effect{f}};
    constexpr auto b = update_text(a, "s"_cts, "r"_cts, "bbb"_cts);
}

TEST(augmented, cumulative_effect) {
    auto f = [](auto acc, auto i, auto s, auto r, auto o) {
        std::cout << i << " " << s << " " << r << " " << o << std::endl;
        return acc + 1;
    };

    auto a = augmented_text{"aaa"_cts, cumulative_effect{f, 0}};
    EXPECT_EQ(a.aux.a, 0);
    auto b = update_text(a, "s"_cts, "r"_cts, "bbb"_cts);
    static_assert(b.text == "bbb"_cts);
    EXPECT_EQ(b.aux.a, 1);
    auto c = update_text(b, "s"_cts, "r"_cts, "ccc"_cts);
    static_assert(c.text == "ccc"_cts);
    EXPECT_EQ(c.aux.a, 2);
}

TEST(augmented, pure_cumulative_effect) {
    constexpr auto f = [](auto acc, auto i, auto s, auto r, auto o) {
        return acc + 1;
    };

    constexpr auto a = augmented_text{"aaa"_cts, cumulative_effect{f, 0}};
    EXPECT_EQ(a.aux.a, 0);
    constexpr auto b = update_text(a, "s"_cts, "r"_cts, "bbb"_cts);
    static_assert(b.text == "bbb"_cts);
    EXPECT_EQ(b.aux.a, 1);
    constexpr auto c = update_text(b, "s"_cts, "r"_cts, "ccc"_cts);
    static_assert(c.text == "ccc"_cts);
    EXPECT_EQ(c.aux.a, 2);
    static_assert(c.aux.a == 2);
}
