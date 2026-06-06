#include "nenormal/nenormal.h"
#include <gtest/gtest.h>
#include "./utils.h"

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

    // inplace

    static_assert(call_inplace_ex(p, std::string(t.value.view()))
        == inplace_argument{std::string(t.value.view()), tristate_kind::not_matched_yet}
    );
    static_assert(
        call_inplace_ex(p,
            inplace_augmented_text{
                std::string(t.value.view()),
                inplace_cumulative_effect{0, inc}
            }
        ) ==
        inplace_argument{
            inplace_augmented_text{
                std::string(t.value.view()),
                inplace_passed{0}
            },
            tristate_kind::not_matched_yet
        }
    );
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

    // inplace

    static_assert(call_inplace_ex(p, std::string(t.value.view()))
        == inplace_argument{std::string(t_one.value.view()), tristate_kind::matched_regular}
    );
    static_assert(
        call_inplace_ex(p,
            inplace_augmented_text{
                std::string(t.value.view()),
                inplace_cumulative_effect{0, inc}
            }
        ) ==
        inplace_argument{
            inplace_augmented_text{
                std::string(t_one.value.view()),
                inplace_passed{0}
            },
            tristate_kind::matched_regular
        }
    );
}

TEST(hidden_rule, final) {
    constexpr auto p = HIDDEN_RULE(FINAL_RULE("c", "d"));
    constexpr auto m = MACHINE(p);

    constexpr auto t_one = CTSTR("aaadcc");
    static_assert(p(not_matched_yet{t}) == matched_final{t_one});
    static_assert(m(t) == t_one);

    static_assert(p(not_matched_yet{at}) == matched_final{augmented_text{t_one, passed{0}}});
    static_assert(m(at) == augmented_text{t_one, passed{0}});

    // inplace

    static_assert(call_inplace_ex(p, std::string(t.value.view()))
        == inplace_argument{std::string(t_one.value.view()), tristate_kind::matched_final}
    );
    static_assert(
        call_inplace_ex(p,
            inplace_augmented_text{
                std::string(t.value.view()),
                inplace_cumulative_effect{0, inc}
            }
        ) ==
        inplace_argument{
            inplace_augmented_text{
                std::string(t_one.value.view()),
                inplace_passed{0}
            },
            tristate_kind::matched_final
        }
    );
}



}} // namespace nn::__unnamed__
