#include <gtest/gtest.h>
#include "nenormal/nenormal.h"

namespace nn {

#define NOT_MATCHED(s) (not_matched_yet{CTSTR(s)})
#define REGULAR(s) (matched_regular{CTSTR(s)})
#define FINAL(s) (matched_final{CTSTR(s)})
#define HALTED(s) (matched_final_halted{CTSTR(s)})

TEST(rule_concepts, acceptance) {
    constexpr auto bare_str = CTSTR("");
    constexpr auto augmented_str = augmented_text{bare_str, empty{}};

    constexpr auto nmy_str = not_matched_yet{bare_str};
    constexpr auto nmy_augmented = not_matched_yet{augmented_str};

    constexpr auto nmy_str_fun = [&]{ return nmy_str; };
    constexpr auto nmy_augmented_fun = [&]{ return nmy_augmented; };

    auto take_rule_nmy_byval = [](RuleInput auto) {};
    static_assert(requires { take_rule_nmy_byval(nmy_str); });
    static_assert(requires { take_rule_nmy_byval(nmy_augmented); });

    auto take_rule_nmy_bycref = [](RuleInput auto const&) {};
    static_assert(requires { take_rule_nmy_bycref(nmy_str); });
    static_assert(requires { take_rule_nmy_bycref(nmy_augmented); });

    static_assert(std::is_same_v<
        decltype(nmy_str_fun()),
        not_matched_yet<decltype(CTSTR(""))>
    >);

    using ttt = std::remove_cvref_t<decltype(nmy_str)>;
    static_assert(RuleInput<ttt>);
    static_assert(RuleInput<ttt const&>);
    static_assert(RuleInput<ttt &&>);

    auto take_rule_nmy_byxref = [](RuleInput auto &&) {};
    static_assert(requires { take_rule_nmy_byxref(nmy_str); });
    static_assert(requires { take_rule_nmy_byxref(nmy_augmented); });
    static_assert(requires { take_rule_nmy_byxref(nmy_str_fun()); });
    static_assert(requires { take_rule_nmy_byxref(nmy_augmented_fun()); });

    RULE("a","b")(nmy_str_fun());
}

TEST(single_rule, fails) {
    static_assert(RULE("a", "b")(NOT_MATCHED("")) == NOT_MATCHED(""));
    static_assert(RULE("a", "b")(NOT_MATCHED("b")) == NOT_MATCHED("b"));
    static_assert(RULE("aaa", "b")(NOT_MATCHED("aa")) == NOT_MATCHED("aa"));
}

TEST(single_rule, when_text_is_same_as_search) {
    static_assert(RULE("foo", "bar")(NOT_MATCHED("foo")) == REGULAR("bar"));
}

TEST(single_rule, when_search_and_replace_differ_by_size) {
    static_assert(RULE("foo", "")(NOT_MATCHED("<<<foo>>>")) == REGULAR("<<<>>>"));
    static_assert(RULE("foo", "br")(NOT_MATCHED("<<<foo>>>")) == REGULAR("<<<br>>>"));
    static_assert(RULE("foo", "bar")(NOT_MATCHED("<<<foo>>>")) == REGULAR("<<<bar>>>"));
    static_assert(RULE("foo", "baar")(NOT_MATCHED("<<<foo>>>")) == REGULAR("<<<baar>>>"));
}

TEST(single_rule, replaces_first_occurence) {
    static_assert(RULE("a", "b")(NOT_MATCHED("aaa")) == REGULAR("baa"));
    static_assert(RULE("a", "b")(NOT_MATCHED("caa")) == REGULAR("cba"));
}

TEST(single_rule, of_final_type) {
    static_assert(FINAL_RULE("a", "b")(NOT_MATCHED("a")) == FINAL("b"));
}

TEST(rules, ctad) {
    constexpr auto program = rules{
        RULE("a", "b"),
        RULE("c", "d"),
        RULE("e", "f"),
    };
    static_assert(program(NOT_MATCHED("abc")) == REGULAR("bbc"));
    static_assert(program(NOT_MATCHED("def")) == REGULAR("dff"));
}

TEST(rules, only_earlier_acts_regular) {
    constexpr auto program = RULES(
        RULE("a", "1"),
        RULE("b", "2"),
        RULE("c", "3")
    );
    static_assert(program(NOT_MATCHED("cbabc")) == REGULAR("cb1bc"));
    static_assert(program(NOT_MATCHED("cbdbc")) == REGULAR("c2dbc"));
    static_assert(program(NOT_MATCHED("cdddc")) == REGULAR("3dddc"));
    static_assert(program(NOT_MATCHED("ddddd")) == NOT_MATCHED("ddddd"));
}

TEST(rules, only_earlier_acts_final) {
    constexpr auto program = RULES(
        FINAL_RULE("a", "1"),
        FINAL_RULE("b", "2"),
        FINAL_RULE("c", "3")
    );
    static_assert(program(NOT_MATCHED("cbabc")) == FINAL("cb1bc"));
    static_assert(program(NOT_MATCHED("cbdbc")) == FINAL("c2dbc"));
    static_assert(program(NOT_MATCHED("cdddc")) == FINAL("3dddc"));
    static_assert(program(NOT_MATCHED("ddddd")) == NOT_MATCHED("ddddd"));
}

TEST(rules, only_earlier_acts_mixed) {
    constexpr auto program = RULES(
        FINAL_RULE("a", "1"),
        RULE("b", "2"),
        FINAL_RULE("b", "Z"),
        FINAL_RULE("c", "3")
    );
    static_assert(program(NOT_MATCHED("cbabc")) == FINAL("cb1bc"));
    static_assert(program(NOT_MATCHED("cbdbc")) == REGULAR("c2dbc"));
    static_assert(program(NOT_MATCHED("cdddc")) == FINAL("3dddc"));
    static_assert(program(NOT_MATCHED("ddddd")) == NOT_MATCHED("ddddd"));
}

TEST(rule_loop, step_by_step) {
    using rule_loop_helpers_ns::rule_loop_body;

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
    static_assert(rl(NOT_MATCHED("aaaaaaaaaaaaaaaa")) == HALTED("bbbbbbbbbbbbbbbb"));
    static_assert(rl(NOT_MATCHED("eaaaaaaaaaaaaaaa")) == FINAL("fbbbbbbbbbbbbbbb"));
}

TEST(augmented, single_rule) {
    constexpr MachineData auto input = augmented_text{CTSTR("aaa"), empty{}};
    using input_type = std::remove_const_t<decltype(input)>;
    static_assert(Augmented<input_type>);
    static_assert(MachineData<input_type>);
    constexpr Rule auto p = RULE("a", "b");
    constexpr RuleMatchedOutput auto output = p(not_matched_yet{input}); // matched_regular{augmented_test{...}}
    using output_type = std::remove_const_t<decltype(output)>;
    constexpr auto output_data = output.value;
    using output_data_type = std::remove_const_t<decltype(output_data)>;
    static_assert(Augmented<output_data_type>);
    static_assert(output_data.text == CTSTR("baa"));
    static_assert(std::same_as<decltype(output_data.aux), empty>);
}

TEST(augmented, moveable_function) {
    // rules should handle fully moveable augmented text,
    // with moveable functions too
    // (lambdas with moveable bound variables, function-like classes, etc.)

    struct moveable {
        moveable() = default;
        moveable(moveable&&) = default;
    };

    auto with_side_effect = [](CtStr auto s) {
        return not_matched_yet{augmented_text{s, side_effect{
            [m = moveable{}](auto...) {}
        }}};
    };
    auto with_cumulative_effect = [](CtStr auto s) {
        return not_matched_yet{augmented_text{s, cumulative_effect{
            moveable{},
            [m = moveable()](auto a, auto...) { return std::move(a); },
        }}};
    };

    auto run_with_effects = [&](Rule auto p, CtStr auto s) {
        p(with_side_effect(s));
        p(with_cumulative_effect(s));
    };
    auto run = [&](Rule auto p) {
        run_with_effects(p, CTSTR("a"));
        run_with_effects(p, CTSTR(""));
    };

    constexpr Rule auto rr = RULE("a", "b");
    constexpr Rule auto fr = FINAL_RULE("a", "b");
    constexpr Rule auto h = HIDDEN_RULE(rr);
    constexpr Rule auto f = FACADE_RULE("facade", rr);

    run(rr);
    run(fr);
    run(h);
    run(f);
}

TEST(single_rule, augmented_fail) {
    constexpr MachineData auto input = augmented_text{CTSTR("aaa"), empty{}};
    constexpr Rule auto p = RULE("c", "d");
    constexpr RuleFailedOutput auto output = p(not_matched_yet{input});
    constexpr MachineData auto output_data = output.value;
    static_assert(input == output_data);
}

TEST(augmented, rules) {
    constexpr auto input = augmented_text{CTSTR("aaa"), empty{}};
    constexpr auto p = RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f"));
    constexpr RuleMatchedOutput auto output = p(not_matched_yet{input});
    static_assert(output.value == augmented_text{CTSTR("baa"), empty{}});
}

TEST(augmented, rules_fail) {
    constexpr auto input = augmented_text{CTSTR("xxx"), empty{}};
    constexpr auto p = RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f"));
    constexpr RuleFailedOutput auto output = p(not_matched_yet{input});
    static_assert(output.value == input);
}

TEST(augmented, rule_loop) {
    constexpr auto input = augmented_text{CTSTR("aaa"), empty{}};
    constexpr auto p = RULE_LOOP(RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f")));
    constexpr RuleMatchedOutput auto output = p(not_matched_yet{input});
    constexpr auto t = output.value.text;
    static_assert(output.value == augmented_text{CTSTR("bbb"), empty{}});
}

TEST(augmented, machine) {
    constexpr auto input = augmented_text{CTSTR("aaa"), empty{}};
    constexpr auto p = MACHINE(RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f")));
    constexpr auto output = p(input);
    static_assert(output.text == CTSTR("bbb"));
}

TEST(augmented, single_rule_runtime) {
    int count = 0;
    auto trace = [&](CtStr auto input, SingleRule auto p, CtStr auto output) {
        ++count;
    };
    auto input = augmented_text{CTSTR("aaa"), side_effect{trace}};
    constexpr auto p = RULE("a", "b");
    p(not_matched_yet{input});
    EXPECT_EQ(count, 1);
}

TEST(augmented, rules_runtime) {
    int count = 0;
    auto trace = [&](CtStr auto input, SingleRule auto p, CtStr auto output) {
        ++count;
    };
    auto input = augmented_text{CTSTR("aaa"), side_effect{trace}};
    constexpr auto p = RULES(RULE("a", "b"), RULE("c", "d"));
    p(not_matched_yet{input});
    EXPECT_EQ(count, 1);
}

TEST(augmented, rule_loop_runtime) {
    int count = 0;
    auto trace = [&](CtStr auto input, SingleRule auto p, CtStr auto output) {
        ++count;
    };
    auto input = augmented_text{CTSTR("aaa"), side_effect{trace}};
    constexpr auto p = RULE_LOOP(RULE("a", "b"));
    auto output = p(not_matched_yet{input});
    static_assert(output.value.text == CTSTR("bbb"));
    EXPECT_EQ(count, 3);
}

TEST(augmented, machine_runtime) {
    int count = 0;
    auto trace = [&](CtStr auto input, SingleRule auto p, CtStr auto output) {
        ++count;
    };
    auto input = augmented_text{CTSTR("aaa"), side_effect{trace}};
    constexpr auto m = MACHINE(RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f")));
    auto output = m(input);
    static_assert(output.text == CTSTR("bbb"));
    EXPECT_EQ(count, 3);
}

TEST(hidden, text) {
    constexpr auto p = HIDDEN_RULE(RULE("a", "b"));
    static_assert(p(NOT_MATCHED("")) == NOT_MATCHED(""));
    static_assert(p(NOT_MATCHED("aaa")) == REGULAR("baa"));
}

TEST(hidden, empty_augmentation) {
    constexpr auto p = HIDDEN_RULE(RULE("a", "b"));

    constexpr MachineData auto bad_input = augmented_text{CTSTR(""), empty{}};
    constexpr RuleFailedOutput auto bad_output = p(not_matched_yet{bad_input});
    static_assert(bad_output.value == bad_input);

    constexpr MachineData auto good_input = augmented_text{CTSTR("aaa"), empty{}};
    constexpr RuleMatchedOutput auto good_output = p(not_matched_yet{good_input});
    static_assert(good_output.value == augmented_text{CTSTR("baa"), empty{}});
}

TEST(hidden, cumulative_augmentation) {
    constexpr auto p = RULE("a", "b");
    constexpr auto h = HIDDEN_RULE(p);

    constexpr auto inc = [](int n, auto...) { return n + 1; };

    constexpr MachineData auto bad_input = augmented_text{CTSTR(""), cumulative_effect{0, inc}};
    constexpr RuleFailedOutput auto bad_output = h(not_matched_yet{bad_input});
    static_assert(bad_output.value == bad_input);

    constexpr MachineData auto good_input = augmented_text{CTSTR("aaa"), cumulative_effect{0, inc}};
    constexpr RuleMatchedOutput auto good_output = h(not_matched_yet{good_input});
    static_assert(good_output.value.text == CTSTR("baa"));
    static_assert(good_output.value.aux == cumulative_effect{0, inc});

    // in contrast, non-hidden rule updates the accumulator
    constexpr RuleMatchedOutput auto good_updated_output = p(not_matched_yet{good_input});
    static_assert(good_updated_output.value.text == CTSTR("baa"));
    static_assert(good_updated_output.value.aux == cumulative_effect{1, inc});
}

/// facade_rule tests

// Test 1: transformation result of bare rule and facade_rule must match

TEST(facade, bare_vs_wrapped_match) {
    constexpr auto p = RULE("a", "b");
    constexpr auto f = FACADE_RULE("my_rule", p);

    static_assert(p(NOT_MATCHED("aaa")) == f(NOT_MATCHED("aaa")));
    static_assert(p(NOT_MATCHED("a")) == f(NOT_MATCHED("a")));
    static_assert(p(NOT_MATCHED("ba")) == f(NOT_MATCHED("ba")));
}

TEST(facade, bare_vs_wrapped_no_match) {
    constexpr auto p = RULE("x", "y");
    constexpr auto f = FACADE_RULE("my_rule", p);

    static_assert(p(NOT_MATCHED("aaa")) == f(NOT_MATCHED("aaa")));
    static_assert(p(NOT_MATCHED("zzz")) == f(NOT_MATCHED("zzz")));
    static_assert(p(NOT_MATCHED("")) == f(NOT_MATCHED("")));
}

TEST(facade, bare_vs_wrapped_final) {
    constexpr auto p = FINAL_RULE("a", "b");
    constexpr auto f = FACADE_RULE("final_rule", p);

    static_assert(p(NOT_MATCHED("aaa")) == f(NOT_MATCHED("aaa")));
    static_assert(p(NOT_MATCHED("a")) == f(NOT_MATCHED("a")));
}

namespace named_rules_ns {

// named rules should be defined in a namespace scope
// and should have distinct names
constexpr auto p_nothing = NAMED_RULE(nothing, RULES());
constexpr auto p_something = NAMED_RULE(something, RULE("a", "b"));

constexpr auto m_nothing = MACHINE(p_nothing);
constexpr auto m_something = MACHINE(p_something);

} // namespace named_rules_ns

TEST(named_rule, ctstr) {
    static_assert(named_rules_ns::m_nothing(CTSTR("aaa")) == CTSTR("aaa"));
    static_assert(named_rules_ns::m_something(CTSTR("aaa")) == CTSTR("bbb"));
}

TEST(named_rule, inplace) {
    EXPECT_EQ(named_rules_ns::m_nothing(std::string{"aaa"}), "aaa");
    EXPECT_EQ(named_rules_ns::m_something(std::string{"aaa"}), "bbb");
}

// Test 2: verify callback receives correct rule type
// If facade_rule is passed to augmented_text with FacadeRule callback,
// it should compile. If bare_rule is passed instead, it should NOT compile.

// Approach: static_assert inside callback with cumulative_effect to verify invocation
TEST(facade, facade_rule_invokes_callback_with_facade_type) {
    // This test verifies that:
    // 1. facade_rule passes FacadeRule check inside callback
    // 2. callback is actually invoked (accumulator changes)
    constexpr auto f = FACADE_RULE("my_rule", RULE("a", "b"));
    constexpr auto inc = [](int acc, CtStr auto, auto rule, CtStr auto) constexpr {
        static_assert(FacadeRule<decltype(rule)>,
            "Callback must receive FacadeRule, not bare rule");
        return acc + 1;
    };
    constexpr MachineData auto input = augmented_text{
        CTSTR("aaa"),
        cumulative_effect{0, inc}
    };
    constexpr auto output = f(not_matched_yet{input});
    static_assert(output.value.aux.a == 1); // callback was invoked, accumulator incremented
}

/// inplace

TEST(inplace, simple_check) {
    constexpr auto p = RULES(RULE("a","b"), FINAL_RULE("c","d"), RULE("e","f"));
    constexpr auto rl = RULE_LOOP(p);
    constexpr auto m = MACHINE_FROM_RULE(rl);

    // pure input - rvalue return, for inline EXPECTs
    constexpr auto run_inplace = [](std::string t, auto&& p) {
        inplace_argument<std::string> a{std::move(t)};
        a.updated_by(p);
        return a;
    };

    // single step
    EXPECT_EQ(
        run_inplace("zzz", p),
        (inplace_argument{"zzz", tristate_kind::not_matched_yet})
    );
    EXPECT_EQ(
        run_inplace("aaa", p),
        (inplace_argument{"baa", tristate_kind::matched_regular})
    );
    EXPECT_EQ(
        run_inplace("ccc", p),
        (inplace_argument{"dcc", tristate_kind::matched_final})
    );
    EXPECT_EQ(
        run_inplace("eee", p),
        (inplace_argument{"fee", tristate_kind::matched_regular})
    );

    // loop
    EXPECT_EQ(
        run_inplace("zzz", rl),
        (inplace_argument{"zzz", tristate_kind::matched_final_halted})
    );
    EXPECT_EQ(
        run_inplace("aaa", rl),
        (inplace_argument{"bbb", tristate_kind::matched_final_halted})
    );
    EXPECT_EQ(
        run_inplace("ccc", rl),
        (inplace_argument{"dcc", tristate_kind::matched_final})
    );
    EXPECT_EQ(
        run_inplace("eee", rl),
        (inplace_argument{"fff", tristate_kind::matched_final_halted})
    );
    EXPECT_EQ(
        run_inplace("aec", rl),
        (inplace_argument{"bed", tristate_kind::matched_final})
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

TEST(inplace, empty) {
    constexpr auto m = MACHINE(RULES(RULE("a","b"), FINAL_RULE("c","d"), RULE("e","f")));
    auto result = m(inplace_augmented_text{"aec", inplace_empty{}});
    EXPECT_EQ(result.text, "bed");
}

TEST(inplace, side_effect) {
    constexpr auto m = MACHINE(RULES(RULE("a","b"), FINAL_RULE("c","d"), RULE("e","f")));
    int counter = 0;
    auto result = m(inplace_augmented_text{
        "aec",
        inplace_side_effect{[&](auto, std::string const&) { ++counter; }}
    });
    EXPECT_EQ(result.text, "bed");
    EXPECT_EQ(counter, 2);
}

TEST(inplace, cumulative_effect) {
    constexpr auto m = MACHINE(RULES(RULE("a","b"), FINAL_RULE("c","d"), RULE("e","f")));
    auto result = m(inplace_augmented_text{
        "aec",
        inplace_cumulative_effect{0, [](int c, auto, std::string const&) { return c + 1; }}
    });
    EXPECT_EQ(result.text, "bed");
    EXPECT_EQ(result.aux.a, 2);
}

TEST(inplace, modification_effect) {
    constexpr auto m = MACHINE(RULES(RULE("a","b"), FINAL_RULE("c","d"), RULE("e","f")));
    auto result = m(inplace_augmented_text{
        "aec",
        inplace_modification_effect{[](int& c, auto, std::string const&) { ++c; }, 0}
    });
    EXPECT_EQ(result.text, "bed");
    EXPECT_EQ(result.aux.a, 2);
}

TEST(inplace, hidden_rule) {
    // hidden_rule does not call the augmentation callback
    constexpr auto m = MACHINE(RULES(
        HIDDEN_RULE(RULE("a","b")),
        HIDDEN_RULE(FINAL_RULE("c","d")),
        HIDDEN_RULE(RULE("e","f"))
    ));
    auto result = m(inplace_augmented_text{
        "aec",
        inplace_modification_effect{[](int& c, auto, std::string const&) { ++c; }, 0}
    });
    EXPECT_EQ(result.text, "bed");
    EXPECT_EQ(result.aux.a, 0);
}

TEST(inplace, facade_rule) {
    // facade_rule calls the callback with the facade_rule itself
    constexpr auto m = MACHINE(RULES(
        FACADE_RULE("r1", RULE("a","b")),
        FACADE_RULE("r2", FINAL_RULE("c","d")),
        FACADE_RULE("r3", RULE("e","f"))
    ));
    auto result = m(inplace_augmented_text{
        "aec",
        inplace_modification_effect{[](int& c, FacadeRule auto rule, std::string const&) { ++c; }, 0}
    });
    EXPECT_EQ(result.text, "bed");
    EXPECT_EQ(result.aux.a, 2);
}

} // namespace nn
