#include "nenormal/nenormal.h"
#include <gtest/gtest.h>
#include <format>
#include <cassert>

namespace test_rule_loop_ns {

// stess testing very long loops

template<size_t N>
constexpr ::nn::CtStr auto dots = ::nn::ct_chars(::nn::ct_size_v<N>, ::nn::ct_char_v<'.'>);

constexpr ::nn::CtStr auto zero = dots<0>;

constexpr ::nn::Machine auto decrement = MACHINE(RULE(".", ""));

void print_step(int step, ::std::string_view o) {
    size_t n = o.size();
    size_t m =
        n > 1000 ? 1000:
        n > 100 ? 100 :
        n > 10 ? 10 :
        1;
    if (n % m == 0)
        ::std::cout << ::std::format("{:6} : {:6}", step, n) << ::std::endl;
}

constexpr auto count_steps =
[](int acc, ::nn::CtStr auto i, ::nn::SingleRule auto const& p, ::nn::CtStr auto o) {
    constexpr std::string_view ov = o.value.view();
    print_step(acc + 1, ov);
    return acc + 1;
};

auto run(::nn::CtStr auto s) {
    int step = 0;
    ::nn::Augmented auto input = ::nn::augmented_text{s, ::nn::cumulative_effect{count_steps, step}};
    ::nn::Augmented auto output = decrement(input);
    std::cout << "-----" << std::endl;
    return output.text.value.size();
};

constexpr auto pure_run(::nn::CtStr auto s) {
    return decrement(s).value.size();
}

TEST(rule_loop, pure) {
    static_assert(pure_run(dots<0>) == 0);
    static_assert(pure_run(dots<1>) == 0);
    static_assert(pure_run(dots<2>) == 0);
    static_assert(pure_run(dots<3>) == 0);
    static_assert(pure_run(dots<4>) == 0);
    static_assert(pure_run(dots<10>) == 0);
    static_assert(pure_run(dots<20>) == 0);
    static_assert(pure_run(dots<100>) == 0);
    static_assert(pure_run(dots<200>) == 0);
}

TEST(rule_loop, run) {
    EXPECT_EQ(run(dots<0>), 0);
    EXPECT_EQ(run(dots<1>), 0);
    EXPECT_EQ(run(dots<10>), 0);
    EXPECT_EQ(run(dots<20>), 0);
    EXPECT_EQ(run(dots<100>), 0);
}

} // namespace test_rule_loop_ns
