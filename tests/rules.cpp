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

    auto take_rule_input = [](RuleInput auto) {};
    static_assert(requires { take_rule_input(bare_str); });
    static_assert(requires { take_rule_input(augmented_str); });

    auto take_rule_nmy_byval = [](RuleNotMatchedYetInput auto) {};
    static_assert(requires { take_rule_nmy_byval(nmy_str); });
    static_assert(requires { take_rule_nmy_byval(nmy_augmented); });

    auto take_rule_nmy_bycref = [](RuleNotMatchedYetInput auto const&) {};
    static_assert(requires { take_rule_nmy_bycref(nmy_str); });
    static_assert(requires { take_rule_nmy_bycref(nmy_augmented); });

    static_assert(std::is_same_v<
        decltype(nmy_str_fun()),
        not_matched_yet<decltype(CTSTR(""))>
    >);

    using ttt = std::remove_cvref_t<decltype(nmy_str)>;
    static_assert(RuleNotMatchedYetInput<ttt>);
    static_assert(RuleNotMatchedYetInput<ttt const&>);
    static_assert(RuleNotMatchedYetInput<ttt &&>);

    auto take_rule_nmy_byxref = [](RuleNotMatchedYetInput auto &&) {};
    static_assert(requires { take_rule_nmy_byxref(nmy_str); });
    static_assert(requires { take_rule_nmy_byxref(nmy_augmented); });
    static_assert(requires { take_rule_nmy_byxref(nmy_str_fun()); });
    static_assert(requires { take_rule_nmy_byxref(nmy_augmented_fun()); });

    RULE("a","b")(nmy_str_fun());
}

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
    constexpr RuleInput auto input = augmented_text{CTSTR("aaa"), empty{}};
    using input_type = std::remove_const_t<decltype(input)>;
    static_assert(Augmented<input_type>);
    static_assert(RuleInput<input_type>);
    constexpr Rule auto p = RULE("a", "b");
    constexpr RuleMatchedOutput auto output = p(input); // matched_regular{augmented_test{...}}
    using output_type = std::remove_const_t<decltype(output)>;
    constexpr auto output_data = output.value;
    using output_data_type = std::remove_const_t<decltype(output_data)>;
    static_assert(Augmented<output_data_type>);
    static_assert(output_data.text == CTSTR("baa"));
    static_assert(std::same_as<decltype(output_data.aux), empty>);
}

TEST(single_rule, augmented_fail) {
    constexpr RuleInput auto input = augmented_text{CTSTR("aaa"), empty{}};
    constexpr Rule auto p = RULE("c", "d");
    constexpr RuleFailedOutput auto output = p(input);
    constexpr RuleInput auto output_data = output.value;
    static_assert(input == output_data);
}

TEST(augmented, rules) {
    constexpr auto input = augmented_text{CTSTR("aaa"), empty{}};
    constexpr auto p = RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f"));
    constexpr RuleMatchedOutput auto output = p(input);
    static_assert(output.value == augmented_text{CTSTR("baa"), empty{}});
}

TEST(augmented, rules_fail) {
    constexpr auto input = augmented_text{CTSTR("xxx"), empty{}};
    constexpr auto p = RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f"));
    constexpr RuleFailedOutput auto output = p(input);
    static_assert(output.value == input);
}

TEST(augmented, rule_loop) {
    constexpr auto input = augmented_text{CTSTR("aaa"), empty{}};
    constexpr auto p = RULE_LOOP(RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f")));
    constexpr RuleMatchedOutput auto output = p(input);
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
    p(input);
    EXPECT_EQ(count, 1);
}

TEST(augmented, rules_runtime) {
    int count = 0;
    auto trace = [&](CtStr auto input, SingleRule auto p, CtStr auto output) {
        ++count;
    };
    auto input = augmented_text{CTSTR("aaa"), side_effect{trace}};
    constexpr auto p = RULES(RULE("a", "b"), RULE("c", "d"));
    p(input);
    EXPECT_EQ(count, 1);
}

TEST(augmented, rule_loop_runtime) {
    int count = 0;
    auto trace = [&](CtStr auto input, SingleRule auto p, CtStr auto output) {
        ++count;
    };
    auto input = augmented_text{CTSTR("aaa"), side_effect{trace}};
    constexpr auto p = RULE_LOOP(RULE("a", "b"));
    auto output = p(input);
    static_assert(output.value.text == CTSTR("bbb"));
    EXPECT_EQ(count, 3);
}

TEST(augmented, machine_runtime) {
    int count = 0;
    auto trace = [&](CtStr auto input, SingleRule auto p, CtStr auto output) {
        ++count;
    };
    auto input = augmented_text{CTSTR("aaa"), side_effect{trace}};
    constexpr auto p = MACHINE(RULES(RULE("c", "d"), RULE("a", "b"), RULE("e", "f")));
    auto output = p(input);
    static_assert(output.text == CTSTR("bbb"));
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

/// facade_rule tests

// Test 1: transformation result of bare rule and facade_rule must match

TEST(facade, bare_vs_wrapped_match) {
    constexpr auto p = RULE("a", "b");
    constexpr auto f = FACADE_RULE("my_rule", p);

    static_assert(p(CTSTR("aaa")) == f(CTSTR("aaa")));
    static_assert(p(CTSTR("a")) == f(CTSTR("a")));
    static_assert(p(CTSTR("ba")) == f(CTSTR("ba")));
}

TEST(facade, bare_vs_wrapped_no_match) {
    constexpr auto p = RULE("x", "y");
    constexpr auto f = FACADE_RULE("my_rule", p);

    static_assert(p(CTSTR("aaa")) == f(CTSTR("aaa")));
    static_assert(p(CTSTR("zzz")) == f(CTSTR("zzz")));
    static_assert(p(CTSTR("")) == f(CTSTR("")));
}

TEST(facade, bare_vs_wrapped_final) {
    constexpr auto p = FINAL_RULE("a", "b");
    constexpr auto f = FACADE_RULE("final_rule", p);

    static_assert(p(CTSTR("aaa")) == f(CTSTR("aaa")));
    static_assert(p(CTSTR("a")) == f(CTSTR("a")));
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
    constexpr RuleInput auto input = augmented_text{
        CTSTR("aaa"),
        cumulative_effect{inc, 0}
    };
    constexpr auto output = f(input);
    static_assert(output.value.aux.a == 1); // callback was invoked, accumulator incremented
}

/// count spurious copy ctors

struct ctor_tracker {
    int copies = 0;
    int moves = 0;
    int inits = 0;

    void copy() { nl(); std::cout << "c"; ++copies; }
    void move() { nl(); std::cout << "m"; ++moves; }
    void init() { nl(); std::cout << "i"; ++inits; }
    void nl() { if ((copies+moves+inits) % 10 == 0) std::cout << "\n    "; }

    friend std::ostream& operator << (std::ostream& os, ctor_tracker const& t) {
        return os << "tracker{c=" << t.copies << ", m=" << t.moves << ", i=" << t.inits << "}";
    }
};
struct ctor_tracker_arg {
    ctor_tracker* t;
    int s = 0;
    explicit ctor_tracker_arg(ctor_tracker* t) noexcept : t{t} {}
    ctor_tracker_arg(ctor_tracker* t, int s) noexcept : t{t}, s{s} {
        t->init();
        show();
    }
    ctor_tracker_arg(const ctor_tracker_arg& o) noexcept : t{o.t}, s{o.s} {
        t->copy();
        show();
    }
    ctor_tracker_arg(ctor_tracker_arg&& o) noexcept : t{o.t}, s{o.s} {
        t->move();
        show();
    }
    void show() {
        std::cout << "[" << s << "]";
    }

    constexpr bool operator == (const ctor_tracker_arg&) const = default;
};


TEST(augmented, spurious_cctors) {
    auto pass = [](ctor_tracker_arg const& a, auto...) {
        return ctor_tracker_arg{a.t, a.s+1};
    };

    auto p_miss = RULE("1","");
    auto p_match = RULE("a","");
    auto p_final = RULE(".", "");
    auto p_mf = RULES(p_match, p_final);
    auto p_miss5 = RULES(p_miss, p_miss, p_miss, p_miss, p_miss);
    auto p_match3 = RULES(RULES(RULES(p_match)));
    auto prog_mismatch = RULES(
        p_miss5,
        HIDDEN_RULE(p_miss5),
        FACADE_RULE("",p_miss5),
        rules<>{}
    );
    auto prog_match = RULES(
        p_miss5,
        p_match,
        p_miss5,
        p_miss5,
        p_miss5
    );

    ctor_tracker t = {};

    const auto nmy = [&](CtStr auto s) {
        return
        not_matched_yet{
            augmented_text{
                s,
                cumulative_effect{
                    pass,
                    ctor_tracker_arg{&t}
                }
            }
        };
    };

    auto examine = [&](const char* title, auto s, auto p, int ec, int em) {
        t = {};
        std::cout << title << " ";
        auto steps = p(nmy(s)).value.aux.a.s;
        std::cout << "\n    " << t << std::endl;
        EXPECT_EQ(t.copies, ec);
        EXPECT_EQ(t.moves, em);
        EXPECT_EQ(t.inits, steps);
        std::cout << std::endl;
    };
    auto examine_m = [&](const char* title, auto s, auto m, int ec, int em) {
        t = {};
        std::cout << title << " ";
        auto steps = m(nmy(s).value).aux.a.s;
        std::cout << "\n    " << t << std::endl;
        EXPECT_EQ(t.copies, ec);
        EXPECT_EQ(t.moves, em);
        EXPECT_EQ(t.inits, steps);
        std::cout << std::endl;
    };

    auto s3 = CTSTR("aaa");

    examine("simple-miss", s3, RULES(RULES(RULES(p_miss))), 0, 0);

    // +1 move per each level of depth from matched rule (return by value)
    examine("simple-match", s3, RULES(RULES(RULES(p_match))), 0, 3);
    examine("match,etc...", s3, RULES(p_match, p_miss, p_miss, p_miss, p_miss), 0, 1);
    examine("many-misses", s3, prog_mismatch, 0, 0);
    examine("miss,match,etc...", s3, RULES(p_miss, p_match, p_miss, p_miss, p_miss), 0, 1);

    // parts of loop
    examine("loop-body-0", s3, rule_loop_body<p_miss>{}, 0, 1);

    // each iteration takes +2 move (rule_loop_body and rule_loop)
    // +1 per each level of nesting (10 iterations or less) - return by value
    examine("mismatch-loop", s3, RULE_LOOP(p_miss5), 0, 2);
    // loop does +1 move per iteration
    examine("match-loop-1",  CTSTR("a"), RULE_LOOP(p_match), 0, 1+2);
    examine("match-loop-3",  CTSTR("aaa"), RULE_LOOP(p_match), 0, 3+2);
    examine("match-loop-9",  CTSTR("aaaaaaaaa"), RULE_LOOP(p_match), 0, 9+2);
    examine("match-loop-10", CTSTR("aaaaaaaaaa"), RULE_LOOP(p_match), 0, 10+3);
    examine("match-loop-11", CTSTR("aaaaaaaaaaa"), RULE_LOOP(p_match), 0, 11+3);
    examine("match-loop-13", CTSTR("aaaaaaaaaaaaa"), RULE_LOOP(p_match), 0, 13+3);
    examine("match-loop-19", CTSTR("aaaaaaaaaaaaaaaaaaa"), RULE_LOOP(p_match), 0, 19+3);
    examine("match-loop-20", CTSTR("aaaaaaaaaaaaaaaaaaaa"), RULE_LOOP(p_match), 0, 20+4);
    examine("match-loop-23", CTSTR("aaaaaaaaaaaaaaaaaaaaaaa"), RULE_LOOP(p_match), 0, 23+4);

    examine("final-loop-!", CTSTR("."), RULE_LOOP(p_final), 0, 1+2);
    // each iteration here takes +2 move (rules, rule_loop_body)
    // each nesting still takes +1 move
    // +1 overall
    examine("final-loop-1",  CTSTR("."), RULE_LOOP(p_mf), 0, 1*2+2);
    examine("final-loop-2",  CTSTR("a."), RULE_LOOP(p_mf), 0, 2*2+2);
    examine("final-loop-3",  CTSTR("aa."), RULE_LOOP(p_mf), 0, 3*2+2);
    examine("final-loop-5",  CTSTR("aaaa."), RULE_LOOP(p_mf), 0, 5*2+2);
    examine("final-loop-9",  CTSTR("aaaaaaaa."), RULE_LOOP(p_mf), 0, 9*2+2);
    examine("final-loop-10", CTSTR("aaaaaaaaa."), RULE_LOOP(p_mf), 0, 10*2+3);
    examine("final-loop-11", CTSTR("aaaaaaaaaa."), RULE_LOOP(p_mf), 0, 11*2+3);
    examine("final-loop-19", CTSTR("aaaaaaaaaaaaaaaaaa."), RULE_LOOP(p_mf), 0, 19*2+3);
    examine("final-loop-20", CTSTR("aaaaaaaaaaaaaaaaaaa."), RULE_LOOP(p_mf), 0, 20*2+4);
    examine("final-loop-23", CTSTR("aaaaaaaaaaaaaaaaaaaaaa."), RULE_LOOP(p_mf), 0, 23*2+4);

    // each iteration here takes +4 move (3 rules and rule_loop_body)
    // each nesting still takes +1 move
    // +1 overall
    examine("deep-loop-1",  CTSTR("a"), RULE_LOOP(p_match3), 0, 1*4+2);
    examine("deep-loop-3",  CTSTR("aaa"), RULE_LOOP(p_match3), 0, 3*4+2);
    examine("deep-loop-5",  CTSTR("aaaaa"), RULE_LOOP(p_match3), 0, 5*4+2);
    examine("deep-loop-9",  CTSTR("aaaaaaaaa"), RULE_LOOP(p_match3), 0, 9*4+2);
    examine("deep-loop-10", CTSTR("aaaaaaaaaa"), RULE_LOOP(p_match3), 0, 10*4+3);
    examine("deep-loop-11", CTSTR("aaaaaaaaaaa"), RULE_LOOP(p_match3), 0, 11*4+3);

    // machine_fun +2 moves
    examine_m("empty-machine", CTSTR("aaa"),    MACHINE_FROM_RULE((rules<>{})), 0, 0+2);
    // machine is machine_fun over rule_loop, so +2 +2 = +4 moves
    examine_m("mismatch-machine", CTSTR("aaa"), MACHINE(p_miss5), 0, 0+4);

    examine_m("match-machine-1",  CTSTR("a"), MACHINE(p_match), 0, 1+4);
    examine_m("match-machine-3",  CTSTR("aaa"), MACHINE(p_match), 0, 3+4);
    examine_m("match-machine-9",  CTSTR("aaaaaaaaa"), MACHINE(p_match), 0, 9+4);
    examine_m("match-machine-10", CTSTR("aaaaaaaaaa"), MACHINE(p_match), 0, 10+5);
    examine_m("match-machine-11", CTSTR("aaaaaaaaaaa"), MACHINE(p_match), 0, 11+5);
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
        inplace_cumulative_effect{[](int c, auto, std::string const&) { return c + 1; }, 0}
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
