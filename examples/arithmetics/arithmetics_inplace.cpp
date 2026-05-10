#include <gtest/gtest.h>
#include "./program.h"

namespace examples::arithmetics {

TEST(arithmetics, inplace) {
    auto q = [](const std::string& s) { return std::quoted(s); };
    auto show = [&](const std::string& src) {
        size_t step = 0;
        auto trace = [&](::nn::Rule auto p, const std::string& dst) {
            using rule_type = decltype(p);
            static_assert(::nn::SingleRule<rule_type> || ::nn::FacadeRule<rule_type>);
            step++;
            std::cout
                << std::setw(3) << step << " : "
                << p
                << "  ==>  "
                << q(dst)
                << std::endl;
        };

        constexpr auto count = [](int n, auto&&...) { return n+1; };

        static_assert(::nn::RuleFixedInput<std::string>);

        auto simple_dst = machine(src);
        std::cout << q(src) << " --> " << q(simple_dst) << std::endl;

        std::cout << q(src) << std::endl;
        auto dst = machine(::nn::inplace_augmented_text{src, ::nn::inplace_side_effect{trace}}).text;
        std::cout << q(dst) << std::endl;
        std::cout << "----- " << step << " steps" << std::endl;
        EXPECT_EQ(dst, simple_dst);

        auto counted = machine(::nn::inplace_augmented_text{src, ::nn::inplace_cumulative_effect{count, 0}}).aux.a;
        EXPECT_EQ(counted, step);
    };

    show("1+2=");
    show("12345+67890=");
    show("98765+66666=");
}

} // namespace examples::arithmetics
