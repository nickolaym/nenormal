#pragma once

#include "nenormal/nenormal.h"

namespace examples::collatz_decimal {

// multiplication works as follows
// let N = nnn * 10 + x
// N*3 = (nnn * 3) * 10 + x * 3
//     = (nnn * 3 + (x*3)/10) + (x*3)%10
//
// <:nnnx *3+a b
// <:nnn *3+c db

constexpr auto mult3plus1 = RULES(
    // start from :>
    RULE("1:>", " *3+0 4>"),
    RULE("3:>", " *3+1 0>"),
    RULE("5:>", " *3+1 6>"),
    RULE("7:>", " *3+2 2>"),
    RULE("9:>", " *3+2 8>"),

    // multiply *3 with carry
    RULE("0 *3+0 ", " *3+0 0"),
    RULE("0 *3+1 ", " *3+0 1"),
    RULE("0 *3+2 ", " *3+0 2"),
    RULE("0 *3+3 ", " *3+0 3"),

    RULE("1 *3+0 ", " *3+0 3"),
    RULE("1 *3+1 ", " *3+0 4"),
    RULE("1 *3+2 ", " *3+0 5"),
    RULE("1 *3+3 ", " *3+0 6"),

    RULE("2 *3+0 ", " *3+0 6"),
    RULE("2 *3+1 ", " *3+0 7"),
    RULE("2 *3+2 ", " *3+0 8"),
    RULE("2 *3+3 ", " *3+0 9"),

    RULE("3 *3+0 ", " *3+0 9"),
    RULE("3 *3+1 ", " *3+1 0"),
    RULE("3 *3+2 ", " *3+1 1"),
    RULE("3 *3+3 ", " *3+1 2"),

    RULE("4 *3+0 ", " *3+1 2"),
    RULE("4 *3+1 ", " *3+1 3"),
    RULE("4 *3+2 ", " *3+1 4"),
    RULE("4 *3+3 ", " *3+1 5"),

    RULE("5 *3+0 ", " *3+1 5"),
    RULE("5 *3+1 ", " *3+1 6"),
    RULE("5 *3+2 ", " *3+1 7"),
    RULE("5 *3+3 ", " *3+1 8"),

    RULE("6 *3+0 ", " *3+1 8"),
    RULE("6 *3+1 ", " *3+1 9"),
    RULE("6 *3+2 ", " *3+2 0"),
    RULE("6 *3+3 ", " *3+2 1"),

    RULE("7 *3+0 ", " *3+2 1"),
    RULE("7 *3+1 ", " *3+2 2"),
    RULE("7 *3+2 ", " *3+2 3"),
    RULE("7 *3+3 ", " *3+2 4"),

    RULE("8 *3+0 ", " *3+2 4"),
    RULE("8 *3+1 ", " *3+2 5"),
    RULE("8 *3+2 ", " *3+2 6"),
    RULE("8 *3+3 ", " *3+2 7"),

    RULE("9 *3+0 ", " *3+2 7"),
    RULE("9 *3+1 ", " *3+2 8"),
    RULE("9 *3+2 ", " *3+2 9"),
    RULE("9 *3+3 ", " *3+3 0"),

    // finish at < (implicit leading 0)
    RULE("< *3+0 ", " +1 <"),
    RULE("< *3+1 ", " +1 <1"),
    RULE("< *3+2 ", " +1 <2"),
    RULE("< *3+3 ", " +1 <3")
);

constexpr auto div2 = RULES(
    // start from :>
    RULE("0:>", " *5+0 >"),
    RULE("2:>", " *5+1 >"),
    RULE("4:>", " *5+2 >"),
    RULE("6:>", " *5+3 >"),
    RULE("8:>", " *5+4 >"),

    // multiply *5 with carry
    RULE("0 *5+0 ", " *5+0 0"),
    RULE("0 *5+1 ", " *5+0 1"),
    RULE("0 *5+2 ", " *5+0 2"),
    RULE("0 *5+3 ", " *5+0 3"),
    RULE("0 *5+4 ", " *5+0 4"),
    RULE("0 *5+5 ", " *5+0 5"),

    RULE("1 *5+0 ", " *5+0 5"),
    RULE("1 *5+1 ", " *5+0 6"),
    RULE("1 *5+2 ", " *5+0 7"),
    RULE("1 *5+3 ", " *5+0 8"),
    RULE("1 *5+4 ", " *5+0 9"),
    RULE("1 *5+5 ", " *5+1 0"),

    RULE("2 *5+0 ", " *5+1 0"),
    RULE("2 *5+1 ", " *5+1 1"),
    RULE("2 *5+2 ", " *5+1 2"),
    RULE("2 *5+3 ", " *5+1 3"),
    RULE("2 *5+4 ", " *5+1 4"),
    RULE("2 *5+5 ", " *5+1 5"),

    RULE("3 *5+0 ", " *5+1 5"),
    RULE("3 *5+1 ", " *5+1 6"),
    RULE("3 *5+2 ", " *5+1 7"),
    RULE("3 *5+3 ", " *5+1 8"),
    RULE("3 *5+4 ", " *5+1 9"),
    RULE("3 *5+5 ", " *5+2 0"),

    RULE("4 *5+0 ", " *5+2 0"),
    RULE("4 *5+1 ", " *5+2 1"),
    RULE("4 *5+2 ", " *5+2 2"),
    RULE("4 *5+3 ", " *5+2 3"),
    RULE("4 *5+4 ", " *5+2 4"),
    RULE("4 *5+5 ", " *5+2 5"),

    RULE("5 *5+0 ", " *5+2 5"),
    RULE("5 *5+1 ", " *5+2 6"),
    RULE("5 *5+2 ", " *5+2 7"),
    RULE("5 *5+3 ", " *5+2 8"),
    RULE("5 *5+4 ", " *5+2 9"),
    RULE("5 *5+5 ", " *5+3 0"),

    RULE("6 *5+0 ", " *5+3 0"),
    RULE("6 *5+1 ", " *5+3 1"),
    RULE("6 *5+2 ", " *5+3 2"),
    RULE("6 *5+3 ", " *5+3 3"),
    RULE("6 *5+4 ", " *5+3 4"),
    RULE("6 *5+5 ", " *5+3 5"),

    RULE("7 *5+0 ", " *5+3 5"),
    RULE("7 *5+1 ", " *5+3 6"),
    RULE("7 *5+2 ", " *5+3 7"),
    RULE("7 *5+3 ", " *5+3 8"),
    RULE("7 *5+4 ", " *5+3 9"),
    RULE("7 *5+5 ", " *5+4 0"),

    RULE("8 *5+0 ", " *5+4 0"),
    RULE("8 *5+1 ", " *5+4 1"),
    RULE("8 *5+2 ", " *5+4 2"),
    RULE("8 *5+3 ", " *5+4 3"),
    RULE("8 *5+4 ", " *5+4 4"),
    RULE("8 *5+5 ", " *5+4 5"),

    RULE("9 *5+0 ", " *5+4 5"),
    RULE("9 *5+1 ", " *5+4 6"),
    RULE("9 *5+2 ", " *5+4 7"),
    RULE("9 *5+3 ", " *5+4 8"),
    RULE("9 *5+4 ", " *5+4 9"),
    RULE("9 *5+5 ", " *5+5 0"),

    // finish at < (implicit leading 0)
    RULE("< *5+0 ", " +1 <"),
    RULE("< *5+1 ", " +1 <1"),
    RULE("< *5+2 ", " +1 <2"),
    RULE("< *5+3 ", " +1 <3"),
    RULE("< *5+4 ", " +1 <4"),
    RULE("< *5+5 ", " +1 <5")
);

constexpr auto mult_or_div = RULES(mult3plus1, div2);

constexpr auto stop_after_iteration = FINAL_RULE(" +1 <", "<");

constexpr auto increment_after_iteration = RULES(
    RULE("0 +1 ", " +0 1"),
    RULE("1 +1 ", " +0 2"),
    RULE("2 +1 ", " +0 3"),
    RULE("3 +1 ", " +0 4"),
    RULE("4 +1 ", " +0 5"),
    RULE("5 +1 ", " +0 6"),
    RULE("6 +1 ", " +0 7"),
    RULE("7 +1 ", " +0 8"),
    RULE("8 +1 ", " +0 9"),
    RULE("9 +1 ", " +1 0"),
    // reached implicit leading 0 at the begin of string
    RULE(" +1 ", " +0 1")
);

constexpr auto stop_after_increment = FINAL_RULE(" +0 ", "");
constexpr auto clean_after_increment = RULE(" +0 ", "");

constexpr auto iteration = RULES(mult3plus1, div2);

constexpr auto stop_on_unit = RULES(
    FINAL_RULE("0<1>", "0"),
    FINAL_RULE("1<1>", "1"),
    FINAL_RULE("2<1>", "2"),
    FINAL_RULE("3<1>", "3"),
    FINAL_RULE("4<1>", "4"),
    FINAL_RULE("5<1>", "5"),
    FINAL_RULE("6<1>", "6"),
    FINAL_RULE("7<1>", "7"),
    FINAL_RULE("8<1>", "8"),
    FINAL_RULE("9<1>", "9"),

    FINAL_RULE("<1>", "0") // immediate stop will return 0
);

constexpr auto start = RULE(">", ":>");

// whole program

constexpr auto program = RULES(
    HIDDEN_RULE(mult_or_div),
    HIDDEN_RULE(increment_after_iteration),
    clean_after_increment,
    stop_on_unit,
    HIDDEN_RULE(start)
);

} // namespace examples::collatz_decimal
