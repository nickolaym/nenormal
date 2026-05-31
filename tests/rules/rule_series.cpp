#include "nenormal/nenormal.h"
#include <gtest/gtest.h>
#include "../utils.h"

namespace nn {

#define NMY(t) not_matched_yet{t}
#define REG(t) matched_regular{t}
#define AE(t) augmented_text{t, empty{}}

struct moveable {
    moveable() = default;
    moveable(moveable&&) = default;
};

constexpr auto mismatched_rule_testcase = [](Rule auto p) {
    constexpr CtStr auto t = CTSTR("");
    using t_t = decltype(CTSTR(""));

    // rvalue-ref on input, rvalue-ref on output
    {
        STATIC_ASSERT_EQ_TYPE(
            decltype( p(NMY(t)) ),
            not_matched_yet<t_t>&&
        );
    }
    // const ref on input, const ref on output
    {
        constexpr auto nmy = NMY(t);
        STATIC_ASSERT_EQ_TYPE(
            decltype( p(nmy) ),
            not_matched_yet<t_t> const&
        );
    }
    // values unchanged
    {
        static_assert(p(NMY(t)) == NMY(t));
        static_assert(p(NMY(AE(t))) == NMY(AE(t)));
    }

    // references unchanged (runtime check)
    {
        RuleInput auto nmy = NMY(t);
        RuleInput auto&& res = p(std::move(nmy));
        EXPECT_EQ(&nmy, &res);
    }
    {
        RuleInput auto const nmy = NMY(t);
        RuleInput auto const& res = p(nmy);
        EXPECT_EQ(&nmy, &res);
    }

    // moveable objects do not require copy construction
    {
        auto f = [m=moveable{}](auto...){};
        p( not_matched_yet{augmented_text{t, side_effect{ std::move(f) }}} );
    }
    {
        auto f = [m=moveable{}](auto...){};
        const auto nmy = not_matched_yet{augmented_text{t, side_effect{ std::move(f) }}};
        p( nmy );
    }
};

TEST(rules, empty) {
    mismatched_rule_testcase(RULES());
}
TEST(rules, mismatched_1) {
    mismatched_rule_testcase(RULES(RULE("a", "x")));
}
TEST(rules, mismatched_multiple) {
    mismatched_rule_testcase(RULES(RULE("a", "x"), RULE("b", "x")));
}
TEST(rules, mismatched_nested) {
    mismatched_rule_testcase(
        RULES(
            RULE("a", "x"),
            RULES(
                RULE("c", "x"),
                RULE("d", "x")
            ),
            RULE("b", "x")
        )
    );
}

constexpr auto matched_rule_testcase = [](Rule auto p) {
    constexpr CtStr auto t = CTSTR("aaa"); // input text
    constexpr CtStr auto e = CTSTR("baa"); // expected result

    using t_t = std::remove_cvref_t<decltype(t)>;
    using e_t = std::remove_cvref_t<decltype(e)>;

    // rvalue-ref on input, rvalue on output
    {
        STATIC_ASSERT_EQ_TYPE(
            decltype( p(NMY(t)) ),
            matched_regular<e_t>
        );
    }
    // const ref on input, const ref on output
    {
        constexpr auto nmy = NMY(t);
        STATIC_ASSERT_EQ_TYPE(
            decltype( p(nmy) ),
            matched_regular<e_t>
        );
    }

    // values updated
    {
        static_assert(p(NMY(t)) == REG(e));
        static_assert(p(NMY(AE(t))) == REG(AE(e)));
    }

    // moveable objects do not require copy construction
    {
        auto f = [m=moveable{}](auto...){};
        p( not_matched_yet{augmented_text{t, side_effect{ std::move(f) }}} );
    }
};

TEST(rules, matched_1) {
    matched_rule_testcase(RULES(RULE("a","b")));
}
TEST(rules, matched_first) {
    matched_rule_testcase(RULES(RULE("a","b"), RULE("a","c")));
}
TEST(rules, matched_middle) {
    matched_rule_testcase(RULES(RULE("x","y"), RULE("a","b"), RULE("a","c")));
}
TEST(rules, matched_nested) {
    matched_rule_testcase(
        RULES(
            RULE("x","y"),
            RULES(
                RULE("x","y"),
                RULE("a","b"),
                RULE("x","y")
            ),
            RULE("a","c")
        )
    );
}

} // namespace nn
