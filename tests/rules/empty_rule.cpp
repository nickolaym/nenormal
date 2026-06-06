#include "nenormal/nenormal.h"
#include <gtest/gtest.h>
#include "./utils.h"

namespace nn { namespace {

constexpr auto p = EMPTY();
constexpr auto m = MACHINE(p);

constexpr auto t = CTSTR("aaa");

TEST(empty_rule, ctstr) {
    static_assert(p(not_matched_yet{t}) == not_matched_yet{t});
    static_assert(m(t) == t);

    static_assert(
        p( not_matched_yet{augmented_text{t, passed{0}}} )
        == not_matched_yet{augmented_text{t, passed{0}}}
    );

    constexpr auto inc = [](int c, auto...) constexpr {
        static_assert(false);
        return c + 1;
    };

    static_assert(
        p( not_matched_yet{augmented_text{t,
            cumulative_effect{0, inc}
        }} )
        == not_matched_yet{augmented_text{t, passed{0}}}
    );

    static_assert(
        m(augmented_text{t, cumulative_effect{0, inc}})
        == augmented_text{t, passed{0}}
    );
}

TEST(empty_rule, inplace) {
    static_assert(call_inplace_ex(p, std::string{"aaa"})
        == inplace_argument{std::string{"aaa"}, tristate_kind::not_matched_yet}
    );
}

}} // namespace nn::__unnamed__
