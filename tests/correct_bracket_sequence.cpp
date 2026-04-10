#include <gtest/gtest.h>

#include "nenormal/nenormal.h"

constexpr auto reduce_pairs = NAMED_RULE(reduce_pairs, RULES(
    RULE("()", ""),
    RULE("[]", ""),
    RULE("{}", "")
));

constexpr auto unify_unpaired = NAMED_RULE(unify_unpaired, RULES(
    RULE("(", "_"),
    RULE("[", "_"),
    RULE("{", "_"),
    RULE(")", "_"),
    RULE("]", "_"),
    RULE("}", "_")
));

constexpr auto shrink_unpaired = NAMED_RULE(shrink_unpaired, RULE("__", "_"));

constexpr auto failure_message = NAMED_RULE(failure_message, RULE("_", "FAILURE"));

constexpr auto program = NAMED_RULE(program, RULES(
    reduce_pairs,
    unify_unpaired,
    shrink_unpaired,
    failure_message
));

constexpr auto machine = rule_loop<program>{};

constexpr auto success = ""_cts;
constexpr auto failure = "FAILURE"_cts;

TEST(program, fails) {
    // nothing to replace
    static_assert(program(""_cts) == fail{});
    static_assert(program("------"_cts) == fail{});
    static_assert(program("FAILURE"_cts) == fail{});
}

TEST(program, single_step) {
    // reduce pairs
    static_assert(program("---()---"_cts) == "------"_cts);
    static_assert(program("---[]---"_cts) == "------"_cts);
    static_assert(program("---{}---"_cts) == "------"_cts);
    // unify unpaired
    static_assert(program("---(---"_cts) == "---_---"_cts);
    static_assert(program("---[---"_cts) == "---_---"_cts);
    static_assert(program("---{---"_cts) == "---_---"_cts);
    static_assert(program("---)---"_cts) == "---_---"_cts);
    static_assert(program("---]---"_cts) == "---_---"_cts);
    static_assert(program("---}---"_cts) == "---_---"_cts);
    // shrink unpaired
    static_assert(program("---__---"_cts) == "---_---"_cts);
    // failure message
    static_assert(program("---_---"_cts) == "---FAILURE---"_cts);
}

TEST(program, priority_pairs_over_unpaired) {
    static_assert(program("---([{+}])+()---"_cts) == "---([{+}])+---"_cts);
    static_assert(program("---([{+}])+{}---"_cts) == "---([{+}])+---"_cts);
    static_assert(program("---([{+}])+[]---"_cts) == "---([{+}])+---"_cts);
}

TEST(machine, no_steps) {
    // nothing to replace - machine stops immediately
    static_assert(machine(""_cts) == ""_cts);
    static_assert(machine("------"_cts) == "------"_cts);
    static_assert(machine("---FAILURE---"_cts) == "---FAILURE---"_cts);
}

TEST(machine, single_step) {
    // exact single step
    // reduce pairs
    static_assert(machine("---()---"_cts) == "------"_cts);
    static_assert(machine("---[]---"_cts) == "------"_cts);
    static_assert(machine("---{}---"_cts) == "------"_cts);

    static_assert(machine("()"_cts) == ""_cts);

    // message
    static_assert(machine("---_---"_cts) == "---FAILURE---"_cts);
}

TEST(machine, single_action) {
    // unpaired
    static_assert(machine("---(---"_cts) == "---FAILURE---"_cts);
    static_assert(machine("---[---"_cts) == "---FAILURE---"_cts);
    static_assert(machine("---{---"_cts) == "---FAILURE---"_cts);
    static_assert(machine("---)---"_cts) == "---FAILURE---"_cts);
    static_assert(machine("---]---"_cts) == "---FAILURE---"_cts);
    static_assert(machine("---}---"_cts) == "---FAILURE---"_cts);
    // unified
    static_assert(machine("---__---"_cts) == "---FAILURE---"_cts);
    static_assert(machine("---___---"_cts) == "---FAILURE---"_cts);
}

TEST(correct_bracket_sequence, empty) {
    static_assert(machine(""_cts) == success);
}

TEST(correct_bracket_sequence, just_round_brackets) {
    static_assert(machine("()"_cts) == success);
    static_assert(machine("(()"_cts) == failure);
    static_assert(machine("())"_cts) == failure);
    static_assert(machine("((()))"_cts) == success);
    static_assert(machine("()()"_cts) == success);
    static_assert(machine("(())()"_cts) == success);
    static_assert(machine("((())"_cts) == failure);
    static_assert(machine("())("_cts) == failure);
}

TEST(correct_bracket_sequence, just_square_brackets) {
    static_assert(machine("[]"_cts) == success);
    static_assert(machine("[["_cts) == failure);
    static_assert(machine("]]"_cts) == failure);
    static_assert(machine("[[]]"_cts) == success);
    static_assert(machine("[][]"_cts) == success);
    static_assert(machine("[[[]]]"_cts) == success);
    static_assert(machine("[]["_cts) == failure);
    static_assert(machine("[]]"_cts) == failure);
}

TEST(correct_bracket_sequence, just_curly_brackets) {
    static_assert(machine("{}"_cts) == success);
    static_assert(machine("{{"_cts) == failure);
    static_assert(machine("}}"_cts) == failure);
    static_assert(machine("{{}}"_cts) == success);
    static_assert(machine("{}"_cts) == success);
    static_assert(machine("{}{}"_cts) == success);
    static_assert(machine("{{{}}}"_cts) == success);
}

TEST(correct_bracket_sequence, mixed_brackets) {
    static_assert(machine("([])"_cts) == success);
    static_assert(machine("[()]"_cts) == success);
    static_assert(machine("({[]})"_cts) == success);
    static_assert(machine("([)]"_cts) == failure);
    static_assert(machine("((["_cts) == failure);
    static_assert(machine("])("_cts) == failure);
    static_assert(machine("{[()]}"_cts) == success);
    static_assert(machine("{[()]}[("_cts) == failure);
    static_assert(machine("([{}])"_cts) == success);
    static_assert(machine("([{"_cts) == failure);
    static_assert(machine(")]}"_cts) == failure);
}

TEST(correct_bracket_sequence, complex_balanced) {
    static_assert(machine("((()))"_cts) == success);
    static_assert(machine("[[[]]]"_cts) == success);
    static_assert(machine("{{{{}}}}"_cts) == success);
    static_assert(machine("(()())"_cts) == success);
    static_assert(machine("([{}])"_cts) == success);
    static_assert(machine("{[()()]}"_cts) == success);
    static_assert(machine("((([])))"_cts) == success);
    static_assert(machine("[({})]"_cts) == success);
    static_assert(machine("{[({})]}"_cts) == success);
}

TEST(correct_bracket_sequence, complex_unbalanced) {
    static_assert(machine("((("_cts) == failure);
    static_assert(machine(")))"_cts) == failure);
    static_assert(machine("[[["_cts) == failure);
    static_assert(machine("]]]"_cts) == failure);
    static_assert(machine("{{{"_cts) == failure);
    static_assert(machine("}}}"_cts) == failure);
    static_assert(machine("(()"_cts) == failure);
    static_assert(machine("())"_cts) == failure);
    static_assert(machine("([)"_cts) == failure);
    static_assert(machine("([]]"_cts) == failure);
    static_assert(machine("{[}]"_cts) == failure);
}

TEST(correct_bracket_sequence, interleaved) {
    static_assert(machine("()[]"_cts) == success);
    static_assert(machine("[]()"_cts) == success);
    static_assert(machine("{}()"_cts) == success);
    static_assert(machine("(){}"_cts) == success);
    static_assert(machine("[]{}"_cts) == success);
    static_assert(machine("{}[]"_cts) == success);
    static_assert(machine("()[]{}"_cts) == success);
    static_assert(machine("{}[]()"_cts) == success);
    static_assert(machine("[]{}()"_cts) == success);
}

TEST(correct_bracket_sequence, deeply_nested_same_type) {
    static_assert(machine("("_cts) == failure);
    static_assert(machine("((("_cts) == failure);
    static_assert(machine("((((("_cts) == failure);
    static_assert(machine(")"_cts) == failure);
    static_assert(machine(")))"_cts) == failure);
    static_assert(machine(")))))"_cts) == failure);
}

TEST(correct_bracket_sequence, alternating_patterns) {
    static_assert(machine(")(("_cts) == failure);
    static_assert(machine("))("_cts) == failure);
    static_assert(machine("()("_cts) == failure);
    static_assert(machine(")()"_cts) == failure);
    static_assert(machine(")()("_cts) == failure);
    static_assert(machine("()())"_cts) == failure);
}
