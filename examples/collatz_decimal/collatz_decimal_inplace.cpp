#include "program_decimal.h"
#include <gtest/gtest.h>

namespace examples::collatz_decimal {

TEST(inplace, complete_run) {
    constexpr auto m = MACHINE(program);

    auto print_text = [](const std::string& t) {
        std::cout << "- " << t << std::endl;
    };
    auto trace = [&](auto&& p, const std::string& t) {
        print_text(t);
    };
    auto run = [&](std::string t) {
        print_text(t);
        auto r = m(::nn::inplace_augmented_text{t, ::nn::inplace_side_effect{trace}}).text;
        std::cout << t << " --> " << r << std::endl << std::endl;
        return t;
    };
    for (const char* t : {"<1>", "<2>", "<3>", "<7>", "<27>"}) {
        run(t);
    }
}

} // namespace examples::collatz_decimal
