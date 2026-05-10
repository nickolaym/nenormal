#include <gtest/gtest.h>
#include "./program.h"

namespace examples::arithmetics {

TEST(arithmetics, runtime) {
    auto q = [](::nn::CtStr auto cts) { return std::quoted(cts.value.view()); };
    auto show = [&](::nn::CtStr auto src) {
        size_t step = 0;
        auto trace = [&](::nn::CtStr auto src, ::nn::Rule auto p, ::nn::CtStr auto dst) {
            step++;
            std::cout
                << std::setw(3) << step << " : "
                << std::setw(20) << std::left << q(src)
                << "  >>  "
                << p
                << "  ==  "
                << q(dst)
                << std::endl;
        };

        constexpr auto count = [](int n, auto...) { return n+1; };

        std::cout << q(src) << std::endl;
        auto dst = machine(::nn::augmented_text{src, ::nn::side_effect{trace}}).text;
        std::cout << q(dst) << std::endl;
        std::cout << "----- " << step << " steps" << std::endl;

        constexpr auto counted = machine(::nn::augmented_text{src, ::nn::cumulative_effect{count, 0}}).aux.a;
        constexpr auto ct_counted = ::nn::ct<counted>{};
        EXPECT_EQ(ct_counted.value, step);
    };

    show(CTSTR("1+2="));
    show(CTSTR("12345+67890="));
    show(CTSTR("98765+66666="));
}

} // namespace examples::arithmetics
