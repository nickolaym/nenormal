#include "nenormal/nenormal.h"
#include <gtest/gtest.h>
#include <format>

namespace examples::collatz_mini_2 {

constexpr auto machine = MACHINE(RULES(
    FINAL_RULE( "^$"          , "ZERO"          ), // stop with zero
    FINAL_RULE( "^1$"         , "STOP"          ), // stop with one
          RULE( "^11"         , "^[run][*2+]11" ), // separated ^, started division
          RULE( "[*2+]11"     , "1[*2+]"        ), // division loop (right to left)
          RULE( "[*2+]$"      , "[+]$"          ), // division done (even), started addition
          RULE( "1[+]"        , "[+]1"          ), // addition loop (right to left)
          RULE( "^[run][+]"   , "^"             ), // addition done, removed separator
          RULE( "[*2+]1$"     , "[*6+]1111$"    ), // division done (odd), started multiplication
          RULE( "1[*6+]"      , "[*6+]111111"   ), // multiplication loop (right to left)
          RULE( "^[run][*6+]" , "^"             ), // multiplication done, removed separator
    RULES() // empty stub
));

auto trace = [](int step, ::nn::CtStr auto src, ::nn::Rule auto p, ::nn::CtStr auto dst) {
    std::cout << std::format("{:3} : {}", step, src.value.view()) << std::endl;
    return step + 1;
};

auto run = [](::nn::CtStr auto src) {
    auto res = machine(::nn::augmented_text{src, ::nn::cumulative_effect{trace, 0}});
    std::cout << std::format("{:3} : {}", res.aux.a, res.text.value.view()) << std::endl;
    return res.text;
};

TEST(collatz_mini_2, n_0) {
    EXPECT_EQ(run(CTSTR("^$")), CTSTR("ZERO"));
}

TEST(collatz_mini_2, n_1) {
    EXPECT_EQ(run(CTSTR("^1$")), CTSTR("STOP"));
}

TEST(collatz_mini_2, n_3) {
    EXPECT_EQ(run(CTSTR("^111$")), CTSTR("STOP"));
}

TEST(collatz_mini_2, n_5) {
    EXPECT_EQ(run(CTSTR("^111$")), CTSTR("STOP"));
}

} // namespace examples::collatz_mini_2

