#include "./either.h"
#include <gtest/gtest.h>

namespace nn {

TEST(either, compare) {
    static_assert(left(1) == left(1));
    static_assert(left(1) != right(1));
    static_assert(left(1) != left(2));
    static_assert(left(left(1)) == left(left(1)));
    static_assert(left(left(1)) != left(1));
    static_assert(left(right(1)) != left(1));
    static_assert(left(right(1)) != right(1));

    static_assert(right(1) == right(1));
    static_assert(right(1) != left(1));
    static_assert(right(1) != right(2));
    static_assert(right(right(1)) == right(right(1)));
    static_assert(right(right(1)) != right(1));
    static_assert(right(left(1)) != right(1));
    static_assert(right(left(1)) != left(1));
}

TEST(either, map) {
    constexpr auto plus10 = [](auto x) { return x + 10; };
    constexpr auto plus20 = [](auto x) { return x + 20; };

    static_assert(left(1).either(plus10, plus20) == 11);
    static_assert(right(1).either(plus10, plus20) == 21);

    static_assert(left(1).eitherLifted(plus10, plus20) == left(11));
    static_assert(right(1).eitherLifted(plus10, plus20) == right(21));
}

TEST(either, chain) {
    constexpr auto plus10l = [](auto x) { return left(x + 10); };
    constexpr auto plus10r = [](auto x) { return right(x + 10); };

    static_assert(left(1) >> plus10l == left(1)); // left values are not applied
    static_assert(right(1) >> plus10l == left(11)); // right values are applied

    static_assert(right(1) >> plus10r == right(11));
    static_assert(right(1) >> plus10r >> plus10r == right(21));
    static_assert(right(1) >> plus10r >> plus10r >> plus10l == left(31));
    static_assert(right(1) >> plus10r >> plus10r >> plus10l >> plus10r == left(31));

    static_assert(left(1) >> plus10r >> plus10r >> plus10l >> plus10r == left(1));
}

} // namespace nn
