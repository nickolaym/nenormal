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

constexpr auto error_message = NAMED_RULE(error_message, FINAL_RULE("_", "ERROR"));
constexpr auto ok_message = NAMED_RULE(ok_message, FINAL_RULE("", "OK"));

constexpr auto program = NAMED_RULE(program, RULES(
    reduce_pairs,
    unify_unpaired,
    shrink_unpaired,
    error_message,
    FINAL_RULE("-", "-"), // for test purposes
    ok_message,
    rules<>{}
));

constexpr auto machine = MACHINE(program);

constexpr auto ok_str = "OK"_cts;
constexpr auto error_str = "ERROR"_cts;

TEST(program, final_step) {
    static_assert(program(""_cts) == matched_final{ok_str});
    static_assert(program("_"_cts) == matched_final{error_str});
}

// note that '-' does not belong to the domain of the program.
// normally it must not appear in the text (will lead to undefined behavior)
// but in single step it shows "some substring before and after the object of replacement"
TEST(program, single_step) {
    // reduce pairs
    static_assert(program("---()---"_cts).value == "------"_cts);
    static_assert(program("---[]---"_cts).value == "------"_cts);
    static_assert(program("---{}---"_cts).value == "------"_cts);
    // unify unpaired
    static_assert(program("---(---"_cts).value == "---_---"_cts);
    static_assert(program("---[---"_cts).value == "---_---"_cts);
    static_assert(program("---{---"_cts).value == "---_---"_cts);
    static_assert(program("---)---"_cts).value == "---_---"_cts);
    static_assert(program("---]---"_cts).value == "---_---"_cts);
    static_assert(program("---}---"_cts).value == "---_---"_cts);
    // shrink unpaired
    static_assert(program("---__---"_cts).value == "---_---"_cts);
    // error_str message
    static_assert(program("---_---"_cts).value == "---ERROR---"_cts);
}

TEST(program, priority_pairs_over_unpaired) {
    static_assert(program("---([{+}])+()---"_cts).value == "---([{+}])+---"_cts);
    static_assert(program("---([{+}])+{}---"_cts).value == "---([{+}])+---"_cts);
    static_assert(program("---([{+}])+[]---"_cts).value == "---([{+}])+---"_cts);
}

TEST(machine, final_step) {
    // nothing to replace - machine stops immediately
    static_assert(machine(""_cts) == ok_str);
    static_assert(machine("_"_cts) == error_str);
}

TEST(machine, single_step) {
    // exact single step
    // reduce pairs
    // (note that we stop due to '-' char)
    static_assert(machine("---()---"_cts) == "------"_cts);
    static_assert(machine("---[]---"_cts) == "------"_cts);
    static_assert(machine("---{}---"_cts) == "------"_cts);

    // failure message
    static_assert(machine("---_---"_cts) == "---ERROR---"_cts);
}

TEST(machine, single_action) {
    // unpaired
    static_assert(machine("---(---"_cts) == "---ERROR---"_cts);
    static_assert(machine("---[---"_cts) == "---ERROR---"_cts);
    static_assert(machine("---{---"_cts) == "---ERROR---"_cts);
    static_assert(machine("---)---"_cts) == "---ERROR---"_cts);
    static_assert(machine("---]---"_cts) == "---ERROR---"_cts);
    static_assert(machine("---}---"_cts) == "---ERROR---"_cts);
    // unified
    static_assert(machine("---__---"_cts) == "---ERROR---"_cts);
    static_assert(machine("---___---"_cts) == "---ERROR---"_cts);
    static_assert(machine("---____---"_cts) == "---ERROR---"_cts);
}

// following tests demonstrate normal use of the machine
// that's why they all are called "correct_bracket_sequence"

TEST(correct_bracket_sequence, empty) {
    static_assert(machine(""_cts) == ok_str);
}

TEST(correct_bracket_sequence, just_round_brackets) {
    static_assert(machine("()"_cts) == ok_str);
    static_assert(machine("(()"_cts) == error_str);
    static_assert(machine("())"_cts) == error_str);
    static_assert(machine("((()))"_cts) == ok_str);
    static_assert(machine("()()"_cts) == ok_str);
    static_assert(machine("(())()"_cts) == ok_str);
    static_assert(machine("((())"_cts) == error_str);
    static_assert(machine("())("_cts) == error_str);
}

TEST(correct_bracket_sequence, just_square_brackets) {
    static_assert(machine("[]"_cts) == ok_str);
    static_assert(machine("[["_cts) == error_str);
    static_assert(machine("]]"_cts) == error_str);
    static_assert(machine("[[]]"_cts) == ok_str);
    static_assert(machine("[][]"_cts) == ok_str);
    static_assert(machine("[[[]]]"_cts) == ok_str);
    static_assert(machine("[]["_cts) == error_str);
    static_assert(machine("[]]"_cts) == error_str);
}

TEST(correct_bracket_sequence, just_curly_brackets) {
    static_assert(machine("{}"_cts) == ok_str);
    static_assert(machine("{{"_cts) == error_str);
    static_assert(machine("}}"_cts) == error_str);
    static_assert(machine("{{}}"_cts) == ok_str);
    static_assert(machine("{}"_cts) == ok_str);
    static_assert(machine("{}{}"_cts) == ok_str);
    static_assert(machine("{{{}}}"_cts) == ok_str);
}

TEST(correct_bracket_sequence, mixed_brackets) {
    static_assert(machine("([])"_cts) == ok_str);
    static_assert(machine("[()]"_cts) == ok_str);
    static_assert(machine("({[]})"_cts) == ok_str);
    static_assert(machine("([)]"_cts) == error_str);
    static_assert(machine("((["_cts) == error_str);
    static_assert(machine("])("_cts) == error_str);
    static_assert(machine("{[()]}"_cts) == ok_str);
    static_assert(machine("{[()]}[("_cts) == error_str);
    static_assert(machine("([{}])"_cts) == ok_str);
    static_assert(machine("([{"_cts) == error_str);
    static_assert(machine(")]}"_cts) == error_str);
}

TEST(correct_bracket_sequence, complex_balanced) {
    static_assert(machine("((()))"_cts) == ok_str);
    static_assert(machine("[[[]]]"_cts) == ok_str);
    static_assert(machine("{{{{}}}}"_cts) == ok_str);
    static_assert(machine("(()())"_cts) == ok_str);
    static_assert(machine("([{}])"_cts) == ok_str);
    static_assert(machine("{[()()]}"_cts) == ok_str);
    static_assert(machine("((([])))"_cts) == ok_str);
    static_assert(machine("[({})]"_cts) == ok_str);
    static_assert(machine("{[({})]}"_cts) == ok_str);
}

TEST(correct_bracket_sequence, complex_unbalanced) {
    static_assert(machine("((("_cts) == error_str);
    static_assert(machine(")))"_cts) == error_str);
    static_assert(machine("[[["_cts) == error_str);
    static_assert(machine("]]]"_cts) == error_str);
    static_assert(machine("{{{"_cts) == error_str);
    static_assert(machine("}}}"_cts) == error_str);
    static_assert(machine("(()"_cts) == error_str);
    static_assert(machine("())"_cts) == error_str);
    static_assert(machine("([)"_cts) == error_str);
    static_assert(machine("([]]"_cts) == error_str);
    static_assert(machine("{[}]"_cts) == error_str);
}

TEST(correct_bracket_sequence, interleaved) {
    static_assert(machine("()[]"_cts) == ok_str);
    static_assert(machine("[]()"_cts) == ok_str);
    static_assert(machine("{}()"_cts) == ok_str);
    static_assert(machine("(){}"_cts) == ok_str);
    static_assert(machine("[]{}"_cts) == ok_str);
    static_assert(machine("{}[]"_cts) == ok_str);
    static_assert(machine("()[]{}"_cts) == ok_str);
    static_assert(machine("{}[]()"_cts) == ok_str);
    static_assert(machine("[]{}()"_cts) == ok_str);
}

TEST(correct_bracket_sequence, deeply_nested_same_type) {
    static_assert(machine("("_cts) == error_str);
    static_assert(machine("((("_cts) == error_str);
    static_assert(machine("((((("_cts) == error_str);
    static_assert(machine(")"_cts) == error_str);
    static_assert(machine(")))"_cts) == error_str);
    static_assert(machine(")))))"_cts) == error_str);
}

TEST(correct_bracket_sequence, alternating_patterns) {
    static_assert(machine(")(("_cts) == error_str);
    static_assert(machine("))("_cts) == error_str);
    static_assert(machine("()("_cts) == error_str);
    static_assert(machine(")()"_cts) == error_str);
    static_assert(machine(")()("_cts) == error_str);
    static_assert(machine("()())"_cts) == error_str);
}
