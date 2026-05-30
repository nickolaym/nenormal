#include "nenormal/augmented.h"
#include "nenormal/inplace/inplace_augmented.h"
#include <gtest/gtest.h>

namespace nn {

using namespace nn::literals;

template<class T> struct foo {
    T t;
    auto rebind(auto u) const { return foo<decltype(u)>{u}; }
};

TEST(augmentation, comparison) {
    static_assert(empty{} == empty{});
    static_assert(passed{123} == passed{123.});

    // type of function must match
    constexpr auto inc = [](auto x, auto...){return x+1;};
    static_assert(cumulative_effect{123, inc} == cumulative_effect{123., inc});
    // can compare with a function-less object
    static_assert(passed{123} == cumulative_effect{123., inc});
    static_assert(cumulative_effect{123, inc} == passed{123.});
}


TEST(augmented, no_augmentation) {
    constexpr auto a = "aaa"_cts;
    constexpr auto b = update_text(a, "rule_goes_here", "bbb"_cts);
    static_assert(b == "bbb"_cts);

    constexpr auto r = rebind_text(b, "rrr"_cts);
    static_assert(r == "rrr"_cts);
}

TEST(augmented, empty) {
    constexpr auto a = augmented_text{"aaa"_cts, empty{}};
    constexpr auto b = update_text(a, "rule_goes_here", "bbb"_cts);
    static_assert(b == augmented_text{"bbb"_cts, empty{}});

    constexpr auto r = rebind_text(b, "rrr"_cts);
    static_assert(r == augmented_text{"rrr"_cts, empty{}});
}

TEST(augmented, passed) {
    constexpr auto a = augmented_text{"aaa"_cts, passed{123}};
    constexpr auto b = update_text(a, "rule_goes_here", "bbb"_cts);
    static_assert(b == augmented_text{"bbb"_cts, passed{123}});

    constexpr auto r = rebind_text(b, "rrr"_cts);
    static_assert(r == augmented_text{"rrr"_cts, passed{123}});
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

    auto a = augmented_text{"aaa"_cts, cumulative_effect{0, f}};
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

    constexpr auto a = augmented_text{"aaa"_cts, cumulative_effect{0, f}};
    static_assert(a.aux.a == 0);
    constexpr auto b = update_text(a, "rule_goes_here", "bbb"_cts);
    static_assert(b == augmented_text{"bbb"_cts, cumulative_effect{1, f}});
    constexpr auto c = update_text(b, "rule_goes_here", "ccc"_cts);
    static_assert(c == augmented_text{"ccc"_cts, cumulative_effect{2, f}});

    constexpr auto r = rebind_text(c, "rrr"_cts);
    static_assert(r == augmented_text{"rrr"_cts, cumulative_effect{2, f}});
}

TEST(augmented_move, empty) {
    auto a = augmented_text{"a"_cts, empty{}};

    auto b = std::move(a).update("rule goes here", "b"_cts);
    static_assert(b == augmented_text{"b"_cts, empty{}});

    auto c = std::move(b).rebind("c"_cts);
    static_assert(c == augmented_text{"c"_cts, empty{}});
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

    auto d = update_text(std::move(c), "rule goes here", "d"_cts);
    auto e = rebind_text(std::move(d), "e"_cts);
    static_assert(e.text == "e"_cts);
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

    auto a = augmented_text{"a"_cts, cumulative_effect{moveable{true}, std::move(f)}};

    auto b = std::move(a).update("rule goes here", "b"_cts);
    static_assert(b.text == "b"_cts);
    EXPECT_TRUE(b.aux.a.valid);

    auto c = std::move(b).rebind("c"_cts);
    static_assert(c.text == "c"_cts);
    EXPECT_FALSE(b.aux.a.valid);
    EXPECT_TRUE(c.aux.a.valid);

    auto d = update_text(std::move(c), "rule goes here", "d"_cts);
    auto e = rebind_text(std::move(d), "e"_cts);
    static_assert(e.text == "e"_cts);
    EXPECT_FALSE(c.aux.a.valid);
    EXPECT_FALSE(d.aux.a.valid);
    EXPECT_TRUE(e.aux.a.valid);
}

TEST(augmented_debug, ctstr) {
    constexpr auto a = CTSTR("a");
    debug_call(a, "hello");
    auto cb = get_debug_callback(a);
    cb("hello");
}

TEST(augmented_debug, simply_augmented) {
    constexpr auto a = augmented_text{CTSTR("a"), empty{}};
    debug_call(a, "hello");
    auto cb = get_debug_callback(a);
    cb("hello");
}

TEST(augmented_debug, debug_augmentation_appropriate) {
    std::string passed;
    const auto d = [&](std::string_view v) {
        passed = v;
    };
    const auto a = augmented_text{CTSTR("a"), debug_augmentation{empty{}, d}};
    debug_call(a, "hello");
    EXPECT_EQ(passed, "hello");
    auto cb = get_debug_callback(a);
    cb("world");
    EXPECT_EQ(passed, "world");
}

TEST(augmented_debug, debug_augmentation_inappropriate) {
    std::string passed;
    const auto d = [&](std::string_view v) {
        passed = v;
    };
    const auto a = augmented_text{CTSTR("a"), debug_augmentation{empty{}, d}};
    debug_call(a, 1, 2, 3);
    EXPECT_EQ(passed, "");
}

/// inplace

TEST(inplace_augmentation, comparison) {
    static_assert(inplace_empty{} == inplace_empty{});
    static_assert(inplace_passed{1} == inplace_passed{1.});

    constexpr auto inc = [](auto x, auto...) { return x+1; };
    static_assert(inplace_cumulative_effect{1, inc} == inplace_cumulative_effect{1, inc});
    static_assert(inplace_cumulative_effect{1, inc} == inplace_passed{1.});
    static_assert(inplace_passed{1} == inplace_cumulative_effect{1., inc});
}

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
    inplace_augmented_text t{"foo", inplace_cumulative_effect{0, f}};
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

} // namespace nn
