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
