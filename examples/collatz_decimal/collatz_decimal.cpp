#include "program_decimal.h"
#include <gtest/gtest.h>

namespace examples::collatz_decimal {

constexpr auto print_text = [](::nn::CtStr auto text) {
    std::cout << "- " << text.value.view() << std::endl;
};

constexpr auto trace = [](auto input, auto p, auto output) {
    print_text(output);
};

constexpr auto show = [](auto prg, ::nn::CtStr auto input) -> ::nn::CtStr auto {
    constexpr auto machine = MACHINE(prg);

    constexpr auto augmented_input = ::nn::augmented_text{input, ::nn::side_effect{trace}};
    print_text(input);
    auto augmented_output = machine(augmented_input);
    std::cout << std::endl;
    return augmented_output.text;
};

TEST(iteration, odd) {
    constexpr auto prg = RULES(mult3plus1, stop_after_iteration);

    // 1-digit (except <1>)
    EXPECT_EQ(show(prg, CTSTR("<3:>")), CTSTR("<10>"));
    EXPECT_EQ(show(prg, CTSTR("<5:>")), CTSTR("<16>"));
    EXPECT_EQ(show(prg, CTSTR("<7:>")), CTSTR("<22>"));
    EXPECT_EQ(show(prg, CTSTR("<9:>")), CTSTR("<28>"));

    // 2-digit
    EXPECT_EQ(show(prg, CTSTR("<11:>")), CTSTR("<34>"));
    EXPECT_EQ(show(prg, CTSTR("<13:>")), CTSTR("<40>"));
    EXPECT_EQ(show(prg, CTSTR("<15:>")), CTSTR("<46>"));
    EXPECT_EQ(show(prg, CTSTR("<17:>")), CTSTR("<52>"));
    EXPECT_EQ(show(prg, CTSTR("<19:>")), CTSTR("<58>"));
    EXPECT_EQ(show(prg, CTSTR("<21:>")), CTSTR("<64>"));
    EXPECT_EQ(show(prg, CTSTR("<23:>")), CTSTR("<70>"));
    EXPECT_EQ(show(prg, CTSTR("<25:>")), CTSTR("<76>"));
    EXPECT_EQ(show(prg, CTSTR("<27:>")), CTSTR("<82>"));
    EXPECT_EQ(show(prg, CTSTR("<29:>")), CTSTR("<88>"));
    EXPECT_EQ(show(prg, CTSTR("<31:>")), CTSTR("<94>"));
    EXPECT_EQ(show(prg, CTSTR("<33:>")), CTSTR("<100>"));
    EXPECT_EQ(show(prg, CTSTR("<35:>")), CTSTR("<106>"));
    EXPECT_EQ(show(prg, CTSTR("<37:>")), CTSTR("<112>"));
    EXPECT_EQ(show(prg, CTSTR("<39:>")), CTSTR("<118>"));
    // ...
    EXPECT_EQ(show(prg, CTSTR("<97:>")), CTSTR("<292>"));
    EXPECT_EQ(show(prg, CTSTR("<99:>")), CTSTR("<298>"));

    // some long
    EXPECT_EQ(show(prg, CTSTR("<56789:>")), CTSTR("<170368>"));
}

TEST(iteration, even) {
    constexpr auto prg = RULES(div2, stop_after_iteration);

    // 1-digit (except 0)
    EXPECT_EQ(show(prg, CTSTR("<2:>")), CTSTR("<1>"));
    EXPECT_EQ(show(prg, CTSTR("<4:>")), CTSTR("<2>"));
    EXPECT_EQ(show(prg, CTSTR("<6:>")), CTSTR("<3>"));
    EXPECT_EQ(show(prg, CTSTR("<8:>")), CTSTR("<4>"));

    // 2-digit
    EXPECT_EQ(show(prg, CTSTR("<10:>")), CTSTR("<5>"));
    EXPECT_EQ(show(prg, CTSTR("<12:>")), CTSTR("<6>"));
    EXPECT_EQ(show(prg, CTSTR("<14:>")), CTSTR("<7>"));
    EXPECT_EQ(show(prg, CTSTR("<16:>")), CTSTR("<8>"));
    EXPECT_EQ(show(prg, CTSTR("<18:>")), CTSTR("<9>"));
    EXPECT_EQ(show(prg, CTSTR("<20:>")), CTSTR("<10>"));
    EXPECT_EQ(show(prg, CTSTR("<22:>")), CTSTR("<11>"));
    EXPECT_EQ(show(prg, CTSTR("<24:>")), CTSTR("<12>"));
    EXPECT_EQ(show(prg, CTSTR("<26:>")), CTSTR("<13>"));
    EXPECT_EQ(show(prg, CTSTR("<28:>")), CTSTR("<14>"));
    EXPECT_EQ(show(prg, CTSTR("<30:>")), CTSTR("<15>"));
    EXPECT_EQ(show(prg, CTSTR("<32:>")), CTSTR("<16>"));
    EXPECT_EQ(show(prg, CTSTR("<34:>")), CTSTR("<17>"));
    EXPECT_EQ(show(prg, CTSTR("<36:>")), CTSTR("<18>"));
    EXPECT_EQ(show(prg, CTSTR("<38:>")), CTSTR("<19>"));
    // ...
    EXPECT_EQ(show(prg, CTSTR("<96:>")), CTSTR("<48>"));
    EXPECT_EQ(show(prg, CTSTR("<98:>")), CTSTR("<49>"));
    EXPECT_EQ(show(prg, CTSTR("<100:>")), CTSTR("<50>"));

    // some long
    EXPECT_EQ(show(prg, CTSTR("<567890:>")), CTSTR("<283945>"));
}

TEST(iteration, mult_or_div) {
    constexpr auto prg = RULES(mult_or_div, stop_after_iteration);

    EXPECT_EQ(show(prg, CTSTR("<56789:>")), CTSTR("<170368>"));
    EXPECT_EQ(show(prg, CTSTR("<567890:>")), CTSTR("<283945>"));
}

TEST(iteration, single_run) {
    constexpr auto prg = RULES(
        mult_or_div,
        stop_after_iteration,
        start
    );

    EXPECT_EQ(show(prg, CTSTR("<56789>")), CTSTR("<170368>"));
    EXPECT_EQ(show(prg, CTSTR("<567890>")), CTSTR("<283945>"));
}

TEST(increment, single_run) {
    constexpr auto prg = RULES(
        increment_after_iteration,
        stop_after_increment
    );
    EXPECT_EQ(show(prg, CTSTR(" +1 ")), CTSTR("1"));
    EXPECT_EQ(show(prg, CTSTR("0 +1 ")), CTSTR("1"));
    EXPECT_EQ(show(prg, CTSTR("1 +1 ")), CTSTR("2"));
    EXPECT_EQ(show(prg, CTSTR("2 +1 ")), CTSTR("3"));
    EXPECT_EQ(show(prg, CTSTR("3 +1 ")), CTSTR("4"));
    EXPECT_EQ(show(prg, CTSTR("4 +1 ")), CTSTR("5"));
    EXPECT_EQ(show(prg, CTSTR("5 +1 ")), CTSTR("6"));
    EXPECT_EQ(show(prg, CTSTR("6 +1 ")), CTSTR("7"));
    EXPECT_EQ(show(prg, CTSTR("7 +1 ")), CTSTR("8"));
    EXPECT_EQ(show(prg, CTSTR("8 +1 ")), CTSTR("9"));
    EXPECT_EQ(show(prg, CTSTR("9 +1 ")), CTSTR("10"));
    EXPECT_EQ(show(prg, CTSTR("1299 +1 ")), CTSTR("1300"));
}

TEST(loop_body, single_run) {
    constexpr auto prg = RULES(
        mult_or_div,
        increment_after_iteration,
        stop_after_increment,
        stop_on_unit,
        start
    );
    EXPECT_EQ(show(prg, CTSTR("<56789>")), CTSTR("1<170368>")); // not counted yet + 1 = 1
    EXPECT_EQ(show(prg, CTSTR("999<567890>")), CTSTR("1000<283945>")); // 999+1 = 1000
    EXPECT_EQ(show(prg, CTSTR("129<1>")), CTSTR("129")); // 129 + nothing = 129
    EXPECT_EQ(show(prg, CTSTR("<1>")), CTSTR("0")); // nothing + nothing = 0
}

TEST(sequence, complete_run) {
    constexpr auto prg = program;

    EXPECT_EQ(show(prg, CTSTR("<1>")), CTSTR("0"));
    EXPECT_EQ(show(prg, CTSTR("<2>")), CTSTR("1"));
    EXPECT_EQ(show(prg, CTSTR("<3>")), CTSTR("7")); // 3-10-5-16-8-4-1
    EXPECT_EQ(show(prg, CTSTR("<7>")), CTSTR("16")); // 7-22-11-34-17-52-13-40-20-10-5-16-8-4-2-1
}

} // namespace examples::collatz_decimal
