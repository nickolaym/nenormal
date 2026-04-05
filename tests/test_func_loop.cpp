#include "../src/func_loop.h"

#include <gtest/gtest.h>


TEST(loop, simple) {
    auto fun = [](auto g, int x) -> int {
        if (x == 0) return 1;
        else return x * g(x - 1);
    };

    // Y = \f . (\g . f(g g)) (\g . f(g g))
    // H = \g . f(g g)
    auto Y = [](auto f) {
        auto fg = [f](auto g, auto... x) {
            auto gg = [g](auto... y) { return g(g, y...); };
            return f(gg, x...);
        };
        auto fgfg = [fg](auto... x) {
            return fg(fg, x...);
        };
        return fgfg;
    };
    auto fact = Y(fun);


    EXPECT_EQ(fact(0), 1);
    EXPECT_EQ(fact(1), 1);
    EXPECT_EQ(fact(5), 1*2*3*4*5);
    // /* constexpr */ auto f = loop_while{
    //     [](int x) { return x > 0; },
    //     [](int x) { return x - 10; },
    // };
    // EXPECT_EQ(f(0), 0);
    // EXPECT_EQ(f(1), -9);
    // EXPECT_EQ(f(105), -5);
}
