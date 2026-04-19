#pragma once

#include "nenormal/nenormal.h"

// input: "<111...11>" with N 1's
// output: "<1>2323232" - it stops with N = 1, logging what it did.

// algorithm:

// first, count pairs, propagating tag "c" to the right
// - <11 -> <:11c - isolating < with :
// - c11 -> 11c
// we end up with <:111...11c> or <:111...11c1>

// even case "c>": put 2 to the log and propagate tag "e" to the left
// - c> -> e>2
// - 11e -> e1
// - :e ->
// we end up with <111...11>2

// odd case "c1>": put 3 to the log and propagate tag "o" to the left
// - c1> -> o1111>3 - 3N+1, that's why we add 4 (2x+1 -> 3x+4)
// - 1o -> o111
// - :o ->
// we end up with <111...11>3

constexpr auto program = RULES(
    // count
    RULE("<1111111111", "<:1111111111c"), // 10 - speed up counting
    RULE("c1111111111", "1111111111c"),
    RULE("<11", "<:11c"),
    RULE("c11", "11c"),
    // even
    RULE("c>", "e>2"),
    RULE("1111111111e", "e11111"), // 10 - speed up
    RULE("11e", "e1"),
    RULE(":e", ""),
    // odd
    RULE("c1>", "o1111>3"),
    RULE("11111o", "o111111111111111"), // 5 - speed up
    RULE("1o", "o111"),
    RULE(":o", ""),
    // final
    FINAL_RULE("<1>", "")
);

constexpr auto machine = MACHINE(program);
