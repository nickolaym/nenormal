#include "nenormal/tristate.h"
#include "nenormal/str.h"
#include <gtest/gtest.h>

constexpr auto mismatch    = [](int x) constexpr { return not_matched_yet{x}; };
constexpr auto regular_inc = [](int x) constexpr { return matched_regular{x + 1}; };
constexpr auto final_inc   = [](int x) constexpr { return matched_final{x + 10}; };

TEST(tristate, not_matched_yet) {
    constexpr auto z = not_matched_yet{0};

    static_assert(z >> mismatch == z);
    static_assert(z >> regular_inc == matched_regular{1});
    static_assert(z >> final_inc == matched_final{10});

    static_assert(z.commit() == matched_final_halted{0});

    static_assert(z.rebind(""_ss) == not_matched_yet{""_ss});
}

TEST(tristate, matched_regular) {
    constexpr auto z = matched_regular{0};

    static_assert(z >> mismatch == z);
    static_assert(z >> regular_inc == z);
    static_assert(z >> final_inc == z);

    static_assert(z.commit() == not_matched_yet{0});

    static_assert(z.rebind(""_ss) == matched_regular{""_ss});
}

TEST(tristate, matched_final) {
    constexpr auto z = matched_final{0};

    static_assert(z >> mismatch == z);
    static_assert(z >> regular_inc == z);
    static_assert(z >> final_inc == z);

    static_assert(z.commit() == z);

    static_assert(z.rebind(""_ss) == matched_final{""_ss});
}

TEST(tristate, matched_final_halted) {
    constexpr auto z = matched_final_halted{0};

    static_assert(z >> mismatch == z);
    static_assert(z >> regular_inc == z);
    static_assert(z >> final_inc == z);

    static_assert(z.commit() == z);

    static_assert(z.rebind(""_ss) == matched_final_halted{""_ss});
}

TEST(tristate, alternatives) {
    constexpr auto z = not_matched_yet{0};

    static_assert(z >> mismatch >> mismatch >> mismatch == z);
    static_assert(z >> mismatch >> regular_inc >> regular_inc == matched_regular{1});
    static_assert(z >> mismatch >> final_inc >> regular_inc == matched_final{10});
}

