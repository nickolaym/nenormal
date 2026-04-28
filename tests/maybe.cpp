#include "nenormal/maybe.h"
#include <gtest/gtest.h>
#include <sstream>
#include <string>

constexpr auto undefined = [](auto... args) {
    static_assert(sizeof...(args) < 0);
};

TEST(maybe, nothing) {
    static_assert(nothing{} == nothing{});
    static_assert(!nothing{});
    static_assert(nothing{}.then(undefined, 123) == 123);
    static_assert(nothing{}.then_else(undefined, []{ return 123; }) == 123);
}

TEST(maybe, just) {
    static_assert(just{123} == just{123});
    static_assert(just{123});
    static_assert(just{123}.then([](auto x){ return -x; }, 456) == -123);
    static_assert(just{123}.then([](auto x){ return -x; }, undefined) == -123);
}

TEST(maybe, compare) {
    static_assert(nothing{} != just{123});
    static_assert(just{123} != nothing{});
    static_assert(just{123} != just{""});
}

TEST(maybe, stringize) {
    auto str = [](auto v) { std::ostringstream ost; ost << v; return ost.str(); };
    EXPECT_EQ(str(nothing{}), "nothing");
    EXPECT_EQ(str(just{123}), "just{123}");
}
