#include <gtest/gtest.h>

#include "./program.h"


TEST(arithmetics, runtime) {
    auto q = [](CtStr auto cts) { return std::quoted(cts.value.view()); };
    auto show = [&](CtStr auto src) {
        size_t step = 0;
        auto trace = [&](CtStr auto src, SingleRule auto p, CtStr auto dst) {
            step++;
            std::cout
                << step << " : "
                << q(src)
                << "  >>  "
                << p
                << "  ==  "
                << q(dst)
                << std::endl;
        };

        constexpr auto count = [](int n, auto...) { return n+1; };

        std::cout << q(src) << std::endl;
        auto dst = machine(augmented_text{src, side_effect{trace}}).text;
        std::cout << q(dst) << std::endl;
        std::cout << "----- " << step << " steps" << std::endl;

        constexpr auto counted = machine(augmented_text{src, cumulative_effect{count, 0}}).aux.a;
        constexpr auto ct_counted = ct<counted>{};
        EXPECT_EQ(ct_counted.value, step);
    };

    show(CTSTR("1+2="));
    show(CTSTR("12345+67890="));
    show(CTSTR("98765+66666="));
}
