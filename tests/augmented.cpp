#include "nenormal/augmented.h"
#include "nenormal/inplace/inplace_augmented.h"
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

TEST(augmented_move, empty) {
    auto a = augmented_text{"a"_cts, empty{}};

    auto b = std::move(a).update("rule goes here", "b"_cts);
    static_assert(b.text == "b"_cts);

    auto c = std::move(b).rebind("c"_cts);
    static_assert(c.text == "c"_cts);
}

TEST(augmented_move, side_effect) {
    struct moveable {
        bool valid = false;
        moveable(bool v) : valid{v} {}
        moveable(moveable&& other) : valid{std::exchange(other.valid, false)} {}
    };
    auto f = [m=moveable(true)](auto...) {
        EXPECT_TRUE(m.valid);
    };

    auto a = augmented_text{"a"_cts, side_effect{std::move(f)}};

    auto b = std::move(a).update("rule goes here", "b"_cts);
    static_assert(b.text == "b"_cts);

    auto c = std::move(b).rebind("c"_cts);
    static_assert(c.text == "c"_cts);
}


TEST(augmented_move, cumulative_effect) {
    struct moveable {
        bool valid = false;
        constexpr moveable(bool v) : valid{v} {}
        constexpr moveable(moveable&& other) : valid{std::exchange(other.valid, false)} {}
    };
    auto f = [m=moveable(true)](moveable a, auto...) {
        EXPECT_TRUE(a.valid);
        EXPECT_TRUE(m.valid);
        return std::move(a);
    };

    auto a = augmented_text{"a"_cts, cumulative_effect{std::move(f), moveable{true}}};

    auto b = std::move(a).update("rule goes here", "b"_cts);
    static_assert(b.text == "b"_cts);
    EXPECT_TRUE(b.aux.a.valid);

    auto c = std::move(b).rebind("c"_cts);
    static_assert(c.text == "c"_cts);
    EXPECT_FALSE(b.aux.a.valid);
    EXPECT_TRUE(c.aux.a.valid);
}

/// inplace

TEST(inplace_augmented, empty) {
    inplace_augmented_text t{"foo", inplace_empty{}};
    EXPECT_EQ(&inplace_extract_text(t), &t.text);
    inplace_update_text(t, "rule goes here");
    EXPECT_EQ(t.text, "foo"); // stays unchanged
}

TEST(inplace_augmented, side_effect) {
    int counter = 0;
    std::string e;
    auto f = [&](auto p, std::string const& t) {
        ++counter;
        e = t;
    };
    inplace_augmented_text t{"foo", inplace_side_effect{f}};
    EXPECT_EQ(&inplace_extract_text(t), &t.text);
    inplace_update_text(t, "rule goes here");
    EXPECT_EQ(t.text, "foo"); // stays unchanged
    EXPECT_EQ(counter, 1);
    EXPECT_EQ(e, "foo");
}

TEST(inplace_augmented, cumulative_effect) {
    auto f = [](int counter, auto p, std::string const& t) {
        return counter + 1;
    };
    inplace_augmented_text t{"foo", inplace_cumulative_effect{f, 0}};
    EXPECT_EQ(&inplace_extract_text(t), &t.text);
    inplace_update_text(t, "rule goes here");
    EXPECT_EQ(t.text, "foo"); // stays unchanged
    EXPECT_EQ(t.aux.a, 1);
}

TEST(inplace_augmented, modification_effect) {
    auto f = [](int& counter, auto p, std::string const& t) {
        ++counter;
    };
    inplace_augmented_text t{"foo", inplace_modification_effect{f, 0}};
    EXPECT_EQ(&inplace_extract_text(t), &t.text);
    inplace_update_text(t, "rule goes here");
    EXPECT_EQ(t.text, "foo"); // stays unchanged
    EXPECT_EQ(t.aux.a, 1);
}
