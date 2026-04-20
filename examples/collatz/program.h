#pragma once

#include "nenormal/nenormal.h"

// input: "<111...11>" with N 1's
// output: "<1>2323232" - it stops with N = 1, logging what it did.

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

// printable rules add some auxillary states to visualize milestones
// and hide intermediate steps

constexpr auto printable_program = RULES(
    // count
    HIDDEN_RULE(RULES(
        RULE("<11", "<:c11"), // start count
        RULE("c1111111111", "1111111111c"),
        RULE("<11", "<:11c"),
        RULE("c11", "11c")
    )),
    // even
    RULE("c>", "c even >"), // debug purposes
    HIDDEN_RULE(RULES(
        RULE("c even >", "e>2"), // start even
        RULE("1111111111e", "e11111"), // 10 - speed up
        RULE("11e", "e1")
    )),
    RULE(":e", " divided "),
    HIDDEN_RULE(RULE(" divided ", "")),
    // odd
    RULE("c1>", "c1 odd >"),
    HIDDEN_RULE(RULES(
        RULE("c1 odd >", "o1111>3"), // start odd
        RULE("11111o", "o111111111111111"), // 5 - speed up
        RULE("1o", "o111")
    )),
    RULE(":o", " multiplied "),
    HIDDEN_RULE(RULE(" multiplied ", "")),
    // final
    FINAL_RULE("<1>", "")
);

constexpr auto printable_machine = MACHINE(printable_program);
