#include "../src/func_compose.h"

#include <type_traits>
#include <concepts>
#include <cstring>

#include <gtest/gtest.h>

TEST(compose, nullary) {
    ss::compose f;
    [](auto a) { static_assert(!requires { f(a); }); } (123);
}

TEST(compose, unary) {
    constexpr ss::compose f{
        [](auto x) requires requires { x*2; } { return x * 2; },
    };
    static_assert(f(123) == 246);
    static_assert(f(123.25) == 246.5);
    [](auto a) { static_assert(!requires { f(a); }); } ("test");
}

TEST(compose, binary) {
    constexpr ss::compose f{
        [](auto x) requires requires { x*2; } { return x * 2; },
        [](auto x) requires requires { strlen(x); } { return strlen(x); },
    };
    static_assert(f(123) == 246);
    static_assert(f(123.25) == 246.5);
    static_assert(f("test") == 4);
    [](auto a) { static_assert(!requires { f(a); }); } ((void*)nullptr);
}

TEST(compose, order_matters) {
    constexpr ss::compose f{
        [](int x) { return x*2; },
        [](double x) { return x*3; },
        [](const char* x) { return strlen(x); }
    };
    static_assert(f(123) == 246);
    static_assert(f(123.99) == 246);  // convertible to int
    static_assert(f("test") == 4);
    [](auto a) { static_assert(!requires { f(a); }); } ((void*)nullptr);
}
