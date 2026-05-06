#include <gtest/gtest.h>
#include "nenormal/nenormal.h"

#define NOT_MATCHED(s) (not_matched_yet{CTSTR(s)})
#define REGULAR(s) (matched_regular{CTSTR(s)})
#define FINAL(s) (matched_final{CTSTR(s)})
#define HALTED(s) (matched_final_halted{CTSTR(s)})

TEST(single_rule, fails) {
    static_assert(RULE("a", "b")(CTSTR("")) == NOT_MATCHED(""));
    static_assert(RULE("a", "b")(CTSTR("b")) == NOT_MATCHED("b"));
    static_assert(RULE("aaa", "b")(CTSTR("aa")) == NOT_MATCHED("aa"));
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

TEST(rules, ctad) {
    constexpr auto program = rules{
        RULE("a", "b"),
        RULE("c", "d"),
        RULE("e", "f"),
    };
    static_assert(program(CTSTR("abc")) == REGULAR("bbc"));
    static_assert(program(CTSTR("def")) == REGULAR("dff"));
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
    static_assert(program(CTSTR("ddddd")) == NOT_MATCHED("ddddd"));
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
    static_assert(program(CTSTR("ddddd")) == NOT_MATCHED("ddddd"));
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
    static_assert(program(CTSTR("ddddd")) == NOT_MATCHED("ddddd"));
}

TEST(rule_loop, step_by_step) {
    constexpr auto rs = RULES(RULE("c", "d"), RULE("a", "b"), FINAL_RULE("e", "f"));
    constexpr auto rb = rule_loop_body<rs>{};
    constexpr auto rl = RULE_LOOP(rs);

    auto expect = [&](auto a0, auto e1, auto e2, auto e3, auto e4, auto e5) {
        constexpr auto a1 = a0 >> rb;
        constexpr auto a2 = a1 >> rb;
        constexpr auto a3 = a2 >> rb;
        constexpr auto a4 = a3 >> rb;
        constexpr auto a5 = a4 >> rb;

        EXPECT_EQ(a1, e1);
        EXPECT_EQ(a2, e2);
        EXPECT_EQ(a3, e3);
        EXPECT_EQ(a4, e4);
        EXPECT_EQ(a5, e5);

        auto ae = a0 >> rl;
        EXPECT_EQ(ae, e5);
        static_assert(ae == e5);
    };
    expect(
        NOT_MATCHED(""),
        HALTED(""),
        HALTED(""),
        HALTED(""),
        HALTED(""),
        HALTED(""));
    expect(
        NOT_MATCHED("a"),
        NOT_MATCHED("b"),
        HALTED("b"),
        HALTED("b"),
        HALTED("b"),
        HALTED("b"));
    expect(
        NOT_MATCHED("aa"),
        NOT_MATCHED("ba"),
        NOT_MATCHED("bb"),
        HALTED("bb"),
        HALTED("bb"),
        HALTED("bb"));
    expect(
        NOT_MATCHED("aaa"),
        NOT_MATCHED("baa"),
        NOT_MATCHED("bba"),
        NOT_MATCHED("bbb"),
        HALTED("bbb"),
        HALTED("bbb"));
    expect(
        NOT_MATCHED("aaae"),
        NOT_MATCHED("baae"),
        NOT_MATCHED("bbae"),
        NOT_MATCHED("bbbe"),
        FINAL("bbbf"),
        FINAL("bbbf"));
    static_assert(rl(CTSTR("aaaaaaaaaaaaaaaa")) == HALTED("bbbbbbbbbbbbbbbb"));
    static_assert(rl(CTSTR("eaaaaaaaaaaaaaaa")) == FINAL("fbbbbbbbbbbbbbbb"));
}

TEST(augmented, single_rule) {
    constexpr RuleInput auto input = augmented_text{"aaa"_cts, empty{}};
    using input_type = std::remove_const_t<decltype(input)>;
    static_assert(Augmented<input_type>);
    static_assert(RuleInput<input_type>);
    constexpr Rule auto p = RULE("a", "b");
    constexpr RuleMatchedOutput auto output = p(input); // matched_regular{augmented_test{...}}
    using output_type = std::remove_const_t<decltype(output)>;
    constexpr auto output_data = output.value;
    using output_data_type = std::remove_const_t<decltype(output_data)>;
    static_assert(Augmented<output_data_type>);
    static_assert(output_data.text == "baa"_cts);
    static_assert(std::same_as<decltype(output_data.aux), empty>);
}

TEST(single_rule, augmented_fail) {
    constexpr RuleInput auto input = augmented_text{"aaa"_cts, empty{}};
    constexpr Rule auto p = RULE("c", "d");
    constexpr RuleFailedOutput auto output = p(input);
    constexpr RuleInput auto output_data = output.value;
    static_assert(input == output_data);
}

TEST(augmented, rules) {
    constexpr auto input = augmented_text{"aaa"_cts, empty{}};
    constexpr auto p = RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f"));
    constexpr RuleMatchedOutput auto output = p(input);
    static_assert(output.value == augmented_text{"baa"_cts, empty{}});
}

TEST(augmented, rules_fail) {
    constexpr auto input = augmented_text{"xxx"_cts, empty{}};
    constexpr auto p = RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f"));
    constexpr RuleFailedOutput auto output = p(input);
    static_assert(output.value == input);
}

TEST(augmented, rule_loop) {
    constexpr auto input = augmented_text{"aaa"_cts, empty{}};
    constexpr auto p = RULE_LOOP(RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f")));
    constexpr RuleMatchedOutput auto output = p(input);
    constexpr auto t = output.value.text;
    static_assert(output.value == augmented_text{"bbb"_cts, empty{}});
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
    static_assert(output.value.text == "bbb"_cts);
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
    static_assert(p(CTSTR("")) == NOT_MATCHED(""));
    static_assert(p(CTSTR("aaa")) == REGULAR("baa"));
}

TEST(hidden, empty_augmentation) {
    constexpr auto p = HIDDEN_RULE(RULE("a", "b"));

    constexpr RuleInput auto bad_input = augmented_text{CTSTR(""), empty{}};
    constexpr RuleFailedOutput auto bad_output = p(bad_input);
    static_assert(bad_output.value == bad_input);

    constexpr RuleInput auto good_input = augmented_text{CTSTR("aaa"), empty{}};
    constexpr RuleMatchedOutput auto good_output = p(good_input);
    static_assert(good_output.value == augmented_text{CTSTR("baa"), empty{}});
}

TEST(hidden, cumulative_augmentation) {
    constexpr auto p = RULE("a", "b");
    constexpr auto h = HIDDEN_RULE(p);

    constexpr auto inc = [](int n, auto...) { return n + 1; };

    constexpr RuleInput auto bad_input = augmented_text{CTSTR(""), cumulative_effect{inc, 0}};
    constexpr RuleFailedOutput auto bad_output = h(bad_input);
    static_assert(bad_output.value == bad_input);

    constexpr RuleInput auto good_input = augmented_text{CTSTR("aaa"), cumulative_effect{inc, 0}};
    constexpr RuleMatchedOutput auto good_output = h(good_input);
    static_assert(good_output.value.text == CTSTR("baa"));
    static_assert(good_output.value.aux == cumulative_effect{inc, 0});

    // in contrast, non-hidden rule updates the accumulator
    constexpr RuleMatchedOutput auto good_updated_output = p(good_input);
    static_assert(good_updated_output.value.text == CTSTR("baa"));
    static_assert(good_updated_output.value.aux == cumulative_effect{inc, 1});
}

/// inplace

constexpr auto run_inplace(std::string t, auto&& p) {
    inplace_argument<std::string> a{std::move(t)};
    a.updated_by(p);
    return a;
}

TEST(inplace, simple_check) {
    constexpr auto p = RULES(RULE("a","b"), FINAL_RULE("c","d"), RULE("e","f"));
    constexpr auto rl = RULE_LOOP(p);
    constexpr auto m = MACHINE_FROM_RULE(rl);

    // single step
    EXPECT_EQ(
        run_inplace("zzz", p),
        (inplace_argument{"zzz", k_not_matched_yet})
    );
    EXPECT_EQ(
        run_inplace("aaa", p),
        (inplace_argument{"baa", k_matched_regular})
    );
    EXPECT_EQ(
        run_inplace("ccc", p),
        (inplace_argument{"dcc", k_matched_final})
    );
    EXPECT_EQ(
        run_inplace("eee", p),
        (inplace_argument{"fee", k_matched_regular})
    );

    // loop
    EXPECT_EQ(
        run_inplace("zzz", rl),
        (inplace_argument{"zzz", k_matched_final_halted})
    );
    EXPECT_EQ(
        run_inplace("aaa", rl),
        (inplace_argument{"bbb", k_matched_final_halted})
    );
    EXPECT_EQ(
        run_inplace("ccc", rl),
        (inplace_argument{"dcc", k_matched_final})
    );
    EXPECT_EQ(
        run_inplace("eee", rl),
        (inplace_argument{"fff", k_matched_final_halted})
    );
    EXPECT_EQ(
        run_inplace("aec", rl),
        (inplace_argument{"bed", k_matched_final})
    );

    // machine
    EXPECT_EQ(
        m(std::string{"zzz"}),
        "zzz"
    );
    EXPECT_EQ(
        m(std::string{"aaa"}),
        "bbb"
    );
    EXPECT_EQ(
        m(std::string{"ccc"}),
        "dcc"
    );
    EXPECT_EQ(
        m(std::string{"eee"}),
        "fff"
    );
    EXPECT_EQ(
        m(std::string{"aec"}),
        "bed"
    );
}
