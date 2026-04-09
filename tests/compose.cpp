#include <gtest/gtest.h>

#include "nenormal/compose.h"
#include "nenormal/ct.h"

template<int D> struct digit_t {
    constexpr auto operator()(arg<int> ax) const { return arg{ax.value * 10 + D}; }
};
template<int D> struct digitstop_t {
    constexpr auto operator()(arg<int> ax) const { return stop{ax.value * 10 + D}; }
};
struct not_a_func_t {}; // no appropriate operator() at all

template<int D> constexpr auto digit = digit_t<D>{};
template<int D> constexpr auto digitstop = digitstop_t<D>{};
constexpr auto not_a_func = not_a_func_t{};

TEST(compose, simple_shifts) {
    static_assert(stop{1} >> digit<2> == stop{1});
    static_assert(stop{1} >> not_a_func == stop{1});
    static_assert(arg{1}  >> digit<2> == arg{12});
    static_assert(arg{1}  >> digit<2> >> digit<3> >> digit<4> == arg{1234});
    static_assert(arg{1}  >> digit<2> >> digitstop<3> >> digit<4> == stop{123});
}

TEST(compose, chain) {
    static_assert(stop{1} >> chain(digit<2>, digit<3>, not_a_func) == stop{1});
    static_assert(arg{1}  >> chain(digit<2>, digit<3>, digit<4>) == arg{1234});
}

TEST(compose, chain_of_chains) {
    static_assert(arg{1} >>
        chain(
            chain(digit<2>, digit<3>, digit<4>),
            chain(digit<5>, digit<6>)
        ) == arg{123456});
}

TEST(compose, repeat) {
    constexpr auto inc = [](arg<int> ax) { return arg{ax.value + 1}; };

    constexpr auto testcase = [inc](CtOf<int> auto ctn) {
        constexpr int n = ctn.value;
        static_assert(arg{0} >> repeat<n>(inc) == arg{n});
    };
    constexpr auto testcases = [testcase](auto... ctns) {
        (testcase(ctns), ...);
    };
    testcases(ctv<0>, ctv<1>, ctv<2>, ctv<3>, ctv<4>);
    testcases(ctv<5>, ctv<6>, ctv<7>, ctv<8>, ctv<9>);
    testcases(ctv<10>, ctv<11>, ctv<12>, ctv<13>, ctv<14>);
    testcases(ctv<15>, ctv<16>, ctv<17>, ctv<18>, ctv<19>);
    testcases(ctv<20>, ctv<25>, ctv<30>, ctv<40>, ctv<50>);
    testcases(ctv<100>, ctv<120>, ctv<123>);
    testcases(ctv<1000>, ctv<10'000>, ctv<50'000>, ctv<100'000>);
}

TEST(compose, repeat_with_side_effect) {
    int x = 0;
    auto inc = [&x](arg<int> ax) {
        x += ax.value;
        if (x % 100 == 0) std::cout << x << std::endl;
        return ax;
    };
    EXPECT_EQ((arg{1} >> repeat<1000>(inc)).value, 1);
    EXPECT_EQ(x, 1000);
}

TEST(compose, endless) {
    auto countdown = []<CtOf<int> T>(arg<T> a) {
        constexpr int n = a.value.value;
        if constexpr (n == 0)
            return stop{0};
        else
            return arg{ctv<n - 1>};
    };
    static_assert(arg{ctv<0>} >> countdown == stop{0});
    static_assert(arg{ctv<1>} >> countdown == arg{ctv<0>});

    static_assert(arg{ctv<123>} >> endless_loop{countdown} == stop{0});

    static_assert(arg{ctv<12345>} >> extending_endless_loop{countdown} == stop{0});
}
