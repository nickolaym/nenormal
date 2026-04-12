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

    static_assert(regular_success.text == text);
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

TEST(rules, empty_rules_always_fail) {
    constexpr auto program = rules<>{};
    static_assert(program(CTSTR("")) == fail{});
}

TEST(rules, singular_rules) {
    constexpr auto program = RULES(
        RULE("a", "b")
    );
    static_assert(program(CTSTR("aaa")) == REGULAR("baa"));
    static_assert(program(CTSTR("c")) == fail{});
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

auto failure_trace = [](CtStr auto text, Rule auto p, CtStr auto dst) {
    ADD_FAILURE() << "no trace expected if there were no match";
};
auto print_trace = [](CtStr auto text, Rule auto p, CtStr auto dst) {
    std::cout
        << p << " ::: "
        << std::quoted(text.value.value)
        << " -> "
        << std::quoted(dst.value.value)
        << std::endl;
};

TEST(runtime, rule_fails) {
    RULE("a", "b")(trace_input{CTSTR(""), failure_trace});
}

TEST(runtime, rules_fails) {
    RULES(
        RULE("a", "b"),
        RULE("c", "d")
    )(trace_input{CTSTR(""), failure_trace});
}

TEST(runtime, rule_trace) {
    bool invoked = false;
    auto trace = [&](CtStr auto text, Rule auto p, CtStr auto dst) {
        invoked = true;
        print_trace(text, p, dst);
    };

    constexpr auto p = RULE("a", "b");

    auto src = CTSTR("abc");
    auto ti = trace_input{src, trace};
    static_assert(ti.value == src);
    p(ti);
    EXPECT_TRUE(invoked);
}

TEST(runtime, rules_trace) {
    bool invoked = false;
    auto trace = [&](CtStr auto text, Rule auto p, CtStr auto dst) {
        invoked = true;
        print_trace(text, p, dst);
    };

    constexpr auto p = RULES(
        RULE("a", "b"),
        RULE("c", "d")
    );

    auto src = CTSTR("abc");
    auto ti = trace_input{src, trace};
    static_assert(ti.value == src);
    p(ti);
    EXPECT_TRUE(invoked);
}

TEST(runtime, loop_trace) {
    int invoked = 0;
    auto trace = [&](CtStr auto text, Rule auto p, CtStr auto dst) {
        invoked++;
        print_trace(text, p, dst);
    };

    // correct bracket sequence
    constexpr auto p = RULES(
        RULE(" ", ""),
        RULE("()", ""),
        RULE("(", "_"),
        RULE(")", "_"),
        RULE("__", "_"),
        FINAL_RULE("_", "INCORRECT"),
        FINAL_RULE("", "CORRECT")
    );
    constexpr auto m = MACHINE(p);

    constexpr auto correct   = CTSTR(" ((((() ())())))  ");
    constexpr auto incorrect = CTSTR(")((((())())()))))(");

    invoked = 0;
    EXPECT_EQ(m(trace_input{correct, trace}), CTSTR("CORRECT"));
    EXPECT_EQ(invoked, 12);

    invoked = 0;
    EXPECT_EQ(m(trace_input{incorrect, trace}), CTSTR("INCORRECT"));
    EXPECT_EQ(invoked, 15);

}
