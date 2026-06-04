#include "nenormal/nenormal.h"
#include <gtest/gtest.h>

namespace nn { namespace {

constexpr auto t = CTSTR("aaaccc");
constexpr auto inc = [](int c, auto...) {
    static_assert(false);  // never called
    return c + 1;
};
constexpr auto at = augmented_text{t, cumulative_effect{0, inc}};

TEST(hidden_rule, mismatched) {
    constexpr auto p = HIDDEN_RULE(RULE("x", "z"));
    constexpr auto m = MACHINE(p);

    static_assert(p(not_matched_yet{t}) == not_matched_yet{t});
    static_assert(m(t) == t);

    static_assert(p(not_matched_yet{at}) == not_matched_yet{augmented_text{t, passed{0}}});
    static_assert(m(at) == augmented_text{t, passed{0}});
}

TEST(hidden_rule, regular) {
    constexpr auto p = HIDDEN_RULE(RULE("a", "b"));
    constexpr auto m = MACHINE(p);

    constexpr auto t_one = CTSTR("baaccc");
    constexpr auto t_all = CTSTR("bbbccc");

    static_assert(p(not_matched_yet{t}) == matched_regular{t_one});
    static_assert(m(t) == t_all);

    static_assert(p(not_matched_yet{at}) == matched_regular{augmented_text{t_one, passed{0}}});
    static_assert(m(at) == augmented_text{t_all, passed{0}});
}

TEST(hidden_rule, final) {
    constexpr auto p = HIDDEN_RULE(FINAL_RULE("c", "d"));
    constexpr auto m = MACHINE(p);

    constexpr auto t_one = CTSTR("aaadcc");
    static_assert(p(not_matched_yet{t}) == matched_final{t_one});
    static_assert(m(t) == t_one);

    static_assert(p(not_matched_yet{at}) == matched_final{augmented_text{t_one, passed{0}}});
    static_assert(m(at) == augmented_text{t_one, passed{0}});
}



}} // namespace nn::__unnamed__
