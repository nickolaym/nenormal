#pragma once

#include "nenormal/nenormal.h"

// input:  "X+Y=" where X and Y are nonnegative decimal numbers
// output: "Z" where Z is nonnegative decimal number (their sum)

// Algorithm:
// consider "Xx+Yy="
// 1) Move x next to y
// 2) add them, giving z or 1z
// 3) shift z out and propagate carry if needed
// repeat until there were no X or no Y.
// Single addition: "Xx+Yy=R" --> "X+U=zR"
// where U is Y+carry, R is the rest digits from previous additions.
// Mathematically it means
// (X*10+x)*10^n     + (Y*10+y)*10^n     +              R
//  X      *10^(n+1) +  Y      *10^(n+1) + (x+y)*10^n + R
//  X      *10^(n+1) + (Y+c   )*10^(n+1) +  z   *10^n + R
// "XXXXXx           +  YYYYYy           =              RRR" - before
// "XXXXX            + uUUUUU            =  z           RRR" - after

// there is empty Y (after series of elementary additions)
// X+=Z
constexpr auto plus_nothing = RULE("+=", "");

// take x and start moving it to the right...
constexpr auto take_digit = RULES(
    RULE("0+", "+[0]"),
    RULE("1+", "+[1]"),
    RULE("2+", "+[2]"),
    RULE("3+", "+[3]"),
    RULE("4+", "+[4]"),
    RULE("5+", "+[5]"),
    RULE("6+", "+[6]"),
    RULE("7+", "+[7]"),
    RULE("8+", "+[8]"),
    RULE("9+", "+[9]"),
    rules<>{}
);

// continue moving x to the right...
constexpr auto move_digit_to_the_right = RULES(
    RULE("[0]0", "0[0]"),
    RULE("[0]1", "1[0]"),
    RULE("[0]2", "2[0]"),
    RULE("[0]3", "3[0]"),
    RULE("[0]4", "4[0]"),
    RULE("[0]5", "5[0]"),
    RULE("[0]6", "6[0]"),
    RULE("[0]7", "7[0]"),
    RULE("[0]8", "8[0]"),
    RULE("[0]9", "9[0]"),

    RULE("[1]0", "0[1]"),
    RULE("[1]1", "1[1]"),
    RULE("[1]2", "2[1]"),
    RULE("[1]3", "3[1]"),
    RULE("[1]4", "4[1]"),
    RULE("[1]5", "5[1]"),
    RULE("[1]6", "6[1]"),
    RULE("[1]7", "7[1]"),
    RULE("[1]8", "8[1]"),
    RULE("[1]9", "9[1]"),

    RULE("[2]0", "0[2]"),
    RULE("[2]1", "1[2]"),
    RULE("[2]2", "2[2]"),
    RULE("[2]3", "3[2]"),
    RULE("[2]4", "4[2]"),
    RULE("[2]5", "5[2]"),
    RULE("[2]6", "6[2]"),
    RULE("[2]7", "7[2]"),
    RULE("[2]8", "8[2]"),
    RULE("[2]9", "9[2]"),

    RULE("[3]0", "0[3]"),
    RULE("[3]1", "1[3]"),
    RULE("[3]2", "2[3]"),
    RULE("[3]3", "3[3]"),
    RULE("[3]4", "4[3]"),
    RULE("[3]5", "5[3]"),
    RULE("[3]6", "6[3]"),
    RULE("[3]7", "7[3]"),
    RULE("[3]8", "8[3]"),
    RULE("[3]9", "9[3]"),

    RULE("[4]0", "0[4]"),
    RULE("[4]1", "1[4]"),
    RULE("[4]2", "2[4]"),
    RULE("[4]3", "3[4]"),
    RULE("[4]4", "4[4]"),
    RULE("[4]5", "5[4]"),
    RULE("[4]6", "6[4]"),
    RULE("[4]7", "7[4]"),
    RULE("[4]8", "8[4]"),
    RULE("[4]9", "9[4]"),

    RULE("[5]0", "0[5]"),
    RULE("[5]1", "1[5]"),
    RULE("[5]2", "2[5]"),
    RULE("[5]3", "3[5]"),
    RULE("[5]4", "4[5]"),
    RULE("[5]5", "5[5]"),
    RULE("[5]6", "6[5]"),
    RULE("[5]7", "7[5]"),
    RULE("[5]8", "8[5]"),
    RULE("[5]9", "9[5]"),

    RULE("[6]0", "0[6]"),
    RULE("[6]1", "1[6]"),
    RULE("[6]2", "2[6]"),
    RULE("[6]3", "3[6]"),
    RULE("[6]4", "4[6]"),
    RULE("[6]5", "5[6]"),
    RULE("[6]6", "6[6]"),
    RULE("[6]7", "7[6]"),
    RULE("[6]8", "8[6]"),
    RULE("[6]9", "9[6]"),

    RULE("[7]0", "0[7]"),
    RULE("[7]1", "1[7]"),
    RULE("[7]2", "2[7]"),
    RULE("[7]3", "3[7]"),
    RULE("[7]4", "4[7]"),
    RULE("[7]5", "5[7]"),
    RULE("[7]6", "6[7]"),
    RULE("[7]7", "7[7]"),
    RULE("[7]8", "8[7]"),
    RULE("[7]9", "9[7]"),

    RULE("[8]0", "0[8]"),
    RULE("[8]1", "1[8]"),
    RULE("[8]2", "2[8]"),
    RULE("[8]3", "3[8]"),
    RULE("[8]4", "4[8]"),
    RULE("[8]5", "5[8]"),
    RULE("[8]6", "6[8]"),
    RULE("[8]7", "7[8]"),
    RULE("[8]8", "8[8]"),
    RULE("[8]9", "9[8]"),

    RULE("[9]0", "0[9]"),
    RULE("[9]1", "1[9]"),
    RULE("[9]2", "2[9]"),
    RULE("[9]3", "3[9]"),
    RULE("[9]4", "4[9]"),
    RULE("[9]5", "5[9]"),
    RULE("[9]6", "6[9]"),
    RULE("[9]7", "7[9]"),
    RULE("[9]8", "8[9]"),
    RULE("[9]9", "9[9]"),

    rules<>{}
);

// "y[x]=" - add x+y with shift-out and carry
constexpr auto add_digits = RULES(
    RULE("0[0]=", "=0"),
    RULE("1[0]=", "=1"),
    RULE("2[0]=", "=2"),
    RULE("3[0]=", "=3"),
    RULE("4[0]=", "=4"),
    RULE("5[0]=", "=5"),
    RULE("6[0]=", "=6"),
    RULE("7[0]=", "=7"),
    RULE("8[0]=", "=8"),
    RULE("9[0]=", "=9"),

    RULE("0[1]=", "=1"),
    RULE("1[1]=", "=2"),
    RULE("2[1]=", "=3"),
    RULE("3[1]=", "=4"),
    RULE("4[1]=", "=5"),
    RULE("5[1]=", "=6"),
    RULE("6[1]=", "=7"),
    RULE("7[1]=", "=8"),
    RULE("8[1]=", "=9"),
    RULE("9[1]=", "^=0"),

    RULE("0[2]=", "=2"),
    RULE("1[2]=", "=3"),
    RULE("2[2]=", "=4"),
    RULE("3[2]=", "=5"),
    RULE("4[2]=", "=6"),
    RULE("5[2]=", "=7"),
    RULE("6[2]=", "=8"),
    RULE("7[2]=", "=9"),
    RULE("8[2]=", "^=0"),
    RULE("9[2]=", "^=1"),

    RULE("0[3]=", "=3"),
    RULE("1[3]=", "=4"),
    RULE("2[3]=", "=5"),
    RULE("3[3]=", "=6"),
    RULE("4[3]=", "=7"),
    RULE("5[3]=", "=8"),
    RULE("6[3]=", "=9"),
    RULE("7[3]=", "^=0"),
    RULE("8[3]=", "^=1"),
    RULE("9[3]=", "^=2"),

    RULE("0[4]=", "=4"),
    RULE("1[4]=", "=5"),
    RULE("2[4]=", "=6"),
    RULE("3[4]=", "=7"),
    RULE("4[4]=", "=8"),
    RULE("5[4]=", "=9"),
    RULE("6[4]=", "^=0"),
    RULE("7[4]=", "^=1"),
    RULE("8[4]=", "^=2"),
    RULE("9[4]=", "^=3"),

    RULE("0[5]=", "=5"),
    RULE("1[5]=", "=6"),
    RULE("2[5]=", "=7"),
    RULE("3[5]=", "=8"),
    RULE("4[5]=", "=9"),
    RULE("5[5]=", "^=0"),
    RULE("6[5]=", "^=1"),
    RULE("7[5]=", "^=2"),
    RULE("8[5]=", "^=3"),
    RULE("9[5]=", "^=4"),

    RULE("0[6]=", "=6"),
    RULE("1[6]=", "=7"),
    RULE("2[6]=", "=8"),
    RULE("3[6]=", "=9"),
    RULE("4[6]=", "^=0"),
    RULE("5[6]=", "^=1"),
    RULE("6[6]=", "^=2"),
    RULE("7[6]=", "^=3"),
    RULE("8[6]=", "^=4"),
    RULE("9[6]=", "^=5"),

    RULE("0[7]=", "=7"),
    RULE("1[7]=", "=8"),
    RULE("2[7]=", "=9"),
    RULE("3[7]=", "^=0"),
    RULE("4[7]=", "^=1"),
    RULE("5[7]=", "^=2"),
    RULE("6[7]=", "^=3"),
    RULE("7[7]=", "^=4"),
    RULE("8[7]=", "^=5"),
    RULE("9[7]=", "^=6"),

    RULE("0[8]=", "=8"),
    RULE("1[8]=", "=9"),
    RULE("2[8]=", "^=0"),
    RULE("3[8]=", "^=1"),
    RULE("4[8]=", "^=2"),
    RULE("5[8]=", "^=3"),
    RULE("6[8]=", "^=4"),
    RULE("7[8]=", "^=5"),
    RULE("8[8]=", "^=6"),
    RULE("9[8]=", "^=7"),

    RULE("0[9]=", "=9"),
    RULE("1[9]=", "^=0"),
    RULE("2[9]=", "^=1"),
    RULE("3[9]=", "^=2"),
    RULE("4[9]=", "^=3"),
    RULE("5[9]=", "^=4"),
    RULE("6[9]=", "^=5"),
    RULE("7[9]=", "^=6"),
    RULE("8[9]=", "^=7"),
    RULE("9[9]=", "^=8"),

    rules<>{}
);

// propagate carry "y^" -> "u" or "^u"
constexpr auto add_carry = RULES(
    RULE("0^", "1"),
    RULE("1^", "2"),
    RULE("2^", "3"),
    RULE("3^", "4"),
    RULE("4^", "5"),
    RULE("5^", "6"),
    RULE("6^", "7"),
    RULE("7^", "8"),
    RULE("8^", "9"),
    RULE("9^", "^0"),

    RULE("+^", "+1"),
    rules<>{}
);

// no more rules, - addition stopped.
constexpr auto cleanup = RULES(
    RULE("+", ""),
    RULE("=", ""),
    rules<>{}
);

constexpr auto program = NAMED_RULE(program_t, RULES(
    FACADE_RULE("carry        x^    : ^y  ", add_carry),
    FACADE_RULE("add digits   [x]y= : ^=z ", add_digits),
    FACADE_RULE("move digit   [x]y  : y[x]", move_digit_to_the_right),
    FACADE_RULE("plus nothing +=    :     ", plus_nothing),
    FACADE_RULE("take digit   x+    : +[x]", take_digit),
    FACADE_RULE("cleanup      +/=   :     ", cleanup),
    FACADE_RULE("that's all folks!        ", FINAL_RULE("", "")),
    rules{}
));

constexpr auto machine = MACHINE(program);
