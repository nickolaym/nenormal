#include <gtest/gtest.h>
#include "nenormal/nenormal.h"

#define REGULAR(s) success{CTSTR(s), ct<regular_state>{}}
#define FINAL(s) success{CTSTR(s), ct<final_state>{}}

TEST(compare_values, str) {
    static_assert(STR("abc") == STR("abc"));
    static_assert(STR("abc") != STR("def"));
    static_assert(CTSTR("abc") == CTSTR("abc"));
    static_assert(CTSTR("abc") != CTSTR("def"));
    static_assert(CTSTR("abc").value == STR("abc"));
}

TEST(compare_values, fail) {
    constexpr auto f = fail{};
    static_assert(f == fail{});

    static_assert(failed(f));
    static_assert(!failed(""_cts));
}

TEST(compare_values, success) {
    constexpr auto text = CTSTR("abc");
    constexpr auto regular_success = success{text, ct<regular_state>{}};
    constexpr auto final_success = success{text, ct<final_state>{}};

    static_assert(regular_success.data == text);
    static_assert(regular_success.state.value == regular_state);
    static_assert(regular_success == REGULAR("abc"));

    static_assert(final_success == FINAL("abc"));
    static_assert(regular_success != final_success);
}

TEST(single_rule, fails) {
    static_assert(RULE("a", "b")(CTSTR("")) == fail{});
    static_assert(RULE("a", "b")(CTSTR("b")) == fail{});
    static_assert(RULE("aaa", "b")(CTSTR("aa")) == fail{});
}

TEST(single_rule, when_text_is_same_as_search) {
    static_assert(RULE("foo", "bar")(CTSTR("foo")) == REGULAR("bar"));
}

TEST(single_rule, when_search_and_replace_differ_by_size) {
    static_assert(RULE("foo", "")(CTSTR("<<<foo>>>")) == REGULAR("<<<>>>"));
    static_assert(RULE("foo", "br")(CTSTR("<<<foo>>>")) == REGULAR("<<<br>>>"));
    static_assert(RULE("foo", "bar")(CTSTR("<<<foo>>>")) == REGULAR("<<<bar>>>"));
    static_assert(RULE("foo", "baar")(CTSTR("<<<foo>>>")) == REGULAR("<<<baar>>>"));
}

TEST(single_rule, replaces_first_occurence) {
    static_assert(RULE("a", "b")(CTSTR("aaa")) == REGULAR("baa"));
    static_assert(RULE("a", "b")(CTSTR("caa")) == REGULAR("cba"));
}

TEST(single_rule, of_final_type) {
    static_assert(FINAL_RULE("a", "b")(CTSTR("a")) == FINAL("b"));
}

TEST(rules, only_earlier_acts_regular) {
    constexpr auto program = RULES(
        RULE("a", "1"),
        RULE("b", "2"),
        RULE("c", "3")
    );
    static_assert(program(CTSTR("cbabc")) == REGULAR("cb1bc"));
    static_assert(program(CTSTR("cbdbc")) == REGULAR("c2dbc"));
    static_assert(program(CTSTR("cdddc")) == REGULAR("3dddc"));
    static_assert(program(CTSTR("ddddd")) == fail{});
}

TEST(rules, only_earlier_acts_final) {
    constexpr auto program = RULES(
        FINAL_RULE("a", "1"),
        FINAL_RULE("b", "2"),
        FINAL_RULE("c", "3")
    );
    static_assert(program(CTSTR("cbabc")) == FINAL("cb1bc"));
    static_assert(program(CTSTR("cbdbc")) == FINAL("c2dbc"));
    static_assert(program(CTSTR("cdddc")) == FINAL("3dddc"));
    static_assert(program(CTSTR("ddddd")) == fail{});
}

TEST(rules, only_earlier_acts_mixed) {
    constexpr auto program = RULES(
        FINAL_RULE("a", "1"),
        RULE("b", "2"),
        FINAL_RULE("b", "Z"),
        FINAL_RULE("c", "3")
    );
    static_assert(program(CTSTR("cbabc")) == FINAL("cb1bc"));
    static_assert(program(CTSTR("cbdbc")) == REGULAR("c2dbc"));
    static_assert(program(CTSTR("cdddc")) == FINAL("3dddc"));
    static_assert(program(CTSTR("ddddd")) == fail{});
}

TEST(augmented, single_rule) {
    constexpr auto input = augmented_text{"aaa"_cts, empty{}};
    using input_type = std::remove_const_t<decltype(input)>;
    static_assert(Augmented<input_type>);
    static_assert(RuleInput<input_type>);
    constexpr auto p = RULE("a", "b");
    constexpr auto output = p(input);
    using output_type = std::remove_const_t<decltype(output)>;
    static_assert(Success<output_type>);
    constexpr auto output_data = output.data;
    using output_data_type = std::remove_const_t<decltype(output_data)>;
    static_assert(Augmented<output_data_type>);
    static_assert(output_data.text == "baa"_cts);
    static_assert(std::same_as<decltype(output_data.aux), empty>);
}

TEST(single_rule, augmented_fail) {
    constexpr auto input = augmented_text{"aaa"_cts, empty{}};
    constexpr auto p = RULE("c", "d");
    constexpr auto output = p(input);
    static_assert(failed(output));
}

TEST(augmented, rules) {
    constexpr auto input = augmented_text{"aaa"_cts, empty{}};
    constexpr auto p = RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f"));
    constexpr auto output = p(input);
    static_assert(!failed(output));
    static_assert(output.data.text == "baa"_cts);
}

TEST(augmented, rules_fail) {
    constexpr auto input = augmented_text{"xxx"_cts, empty{}};
    constexpr auto p = RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f"));
    constexpr auto output = p(input);
    static_assert(failed(output));
}

TEST(augmented, rule_loop) {
    constexpr auto input = augmented_text{"aaa"_cts, empty{}};
    constexpr auto p = RULE_LOOP(RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f")));
    constexpr auto output = p(input);
    static_assert(!failed(output));
    static_assert(output.data.text == "bbb"_cts);
}

TEST(augmented, machine) {
    constexpr auto input = augmented_text{"aaa"_cts, empty{}};
    constexpr auto p = MACHINE(RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f")));
    constexpr auto output = p(input);
    static_assert(output.text == "bbb"_cts);
}

TEST(augmented, single_rule_runtime) {
    int count = 0;
    auto trace = [&](CtStr auto input, SingleRule auto p, CtStr auto output) {
        ++count;
    };
    auto input = augmented_text{"aaa"_cts, side_effect{trace}};
    constexpr auto p = RULE("a", "b");
    p(input);
    EXPECT_EQ(count, 1);
}

TEST(augmented, rules_runtime) {
    int count = 0;
    auto trace = [&](CtStr auto input, SingleRule auto p, CtStr auto output) {
        ++count;
    };
    auto input = augmented_text{"aaa"_cts, side_effect{trace}};
    constexpr auto p = RULES(RULE("a", "b"), RULE("c", "d"));
    p(input);
    EXPECT_EQ(count, 1);
}

TEST(augmented, rule_loop_runtime) {
    int count = 0;
    auto trace = [&](CtStr auto input, SingleRule auto p, CtStr auto output) {
        ++count;
    };
    auto input = augmented_text{"aaa"_cts, side_effect{trace}};
    constexpr auto p = RULE_LOOP(RULE("a", "b"));
    auto output = p(input);
    static_assert(output.data.text == "bbb"_cts);
    EXPECT_EQ(count, 3);
}

TEST(augmented, machine_runtime) {
    int count = 0;
    auto trace = [&](CtStr auto input, SingleRule auto p, CtStr auto output) {
        ++count;
    };
    auto input = augmented_text{"aaa"_cts, side_effect{trace}};
    constexpr auto p = MACHINE(RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f")));
    auto output = p(input);
    static_assert(output.text == "bbb"_cts);
    EXPECT_EQ(count, 3);
}

TEST(hidden, text) {
    constexpr auto p = HIDDEN_RULE(RULE("a", "b"));
    static_assert(failed(p(CTSTR(""))));
    static_assert(p(CTSTR("aaa")).data == CTSTR("baa"));
}

TEST(hidden, empty_augmentation) {
    constexpr auto p = HIDDEN_RULE(RULE("a", "b"));

    constexpr RuleInput auto bad_input = augmented_text{CTSTR(""), empty{}};
    constexpr RuleOutput auto bad_output = p(bad_input);
    static_assert(failed(bad_output));

    constexpr RuleInput auto good_input = augmented_text{CTSTR("aaa"), empty{}};
    constexpr RuleOutput auto good_output = p(good_input);
    static_assert(good_output.data.text == CTSTR("baa"));
}

TEST(hidden, cumulative_augmentation) {
    constexpr auto p = RULE("a", "b");
    constexpr auto h = HIDDEN_RULE(p);

    constexpr auto inc = [](int n, auto...) { return n + 1; };

    constexpr RuleInput auto bad_input = augmented_text{CTSTR(""), cumulative_effect{inc, 0}};
    constexpr RuleOutput auto bad_output = h(bad_input);
    static_assert(failed(bad_output));

    constexpr RuleInput auto good_input = augmented_text{CTSTR("aaa"), cumulative_effect{inc, 0}};
    constexpr RuleOutput auto good_output = h(good_input);
    static_assert(good_output.data.text == CTSTR("baa"));
    static_assert(good_output.data.aux.a == 0);

    // in contrast, non-hidden rule updates the accumulator
    constexpr RuleOutput auto good_updated_output = p(good_input);
    static_assert(good_updated_output.data.text == CTSTR("baa"));
    static_assert(good_updated_output.data.aux.a == 1);
}
