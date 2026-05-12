#include "nenormal/tristate.h"
#include "nenormal/inplace/inplace_tristate.h"
#include "nenormal/str.h"
#include <gtest/gtest.h>

namespace nn {

using namespace nn::literals;

TEST(tristate, concepts) {
    using nmy_t = not_matched_yet<int>;
    using reg_t = matched_regular<int>;
    using fin_t = matched_final<int>;
    using hlt_t = matched_final_halted<int>;

    static_assert(Tristate<nmy_t>);
    static_assert(NotMatchedYet<nmy_t>);
    static_assert(!MatchedRegular<nmy_t>);
    static_assert(!MatchedFinal<nmy_t>);

    static_assert(Tristate<reg_t>);
    static_assert(!NotMatchedYet<reg_t>);
    static_assert(MatchedRegular<reg_t>);
    static_assert(!MatchedFinal<reg_t>);

    static_assert(Tristate<fin_t>);
    static_assert(!NotMatchedYet<fin_t>);
    static_assert(!MatchedRegular<fin_t>);
    static_assert(MatchedFinal<fin_t>);

    static_assert(Tristate<hlt_t>);
    static_assert(!NotMatchedYet<hlt_t>);
    static_assert(!MatchedRegular<hlt_t>);
    static_assert(MatchedFinal<hlt_t>);
}

constexpr auto mismatch    = [](auto&& x) constexpr -> decltype(auto) { return std::forward<decltype(x)>(x); };
constexpr auto regular_inc = [](auto&& x) constexpr { return matched_regular{x.value + 1}; };
constexpr auto final_inc   = [](auto&& x) constexpr { return matched_final{x.value + 10}; };

TEST(tristate, not_matched_yet) {
    constexpr auto z = not_matched_yet{0};

    // conventional mismatch function returns reference to its arg
    static_assert(std::is_same_v< decltype(z >> mismatch), not_matched_yet<int> const& >);
    static_assert(&(z >> mismatch) == &z);
    static_assert(z >> regular_inc == matched_regular{1});
    static_assert(z >> final_inc == matched_final{10});

    // commit returns reference
    static_assert(&(z.commit_alts()) == &z);

    static_assert(z.commit_loop() == matched_final_halted{0});

    static_assert(z.rebind(""_ss) == not_matched_yet{""_ss});
}

TEST(tristate, matched_regular) {
    constexpr auto z = matched_regular{0};

    // >> skips right arg and returns reference to itself
    static_assert(&(z >> mismatch) == &z);
    static_assert(&(z >> regular_inc) == &z);
    static_assert(&(z >> final_inc) == &z);

    // commit_alt returns value (because z itself is temporary)
    static_assert(z.commit_alts() == z);
    static_assert(std::is_same_v< decltype(z.commit_alts()), matched_regular<int> >);

    static_assert(z.commit_loop() == not_matched_yet{0});

    static_assert(z.rebind(""_ss) == matched_regular{""_ss});
}

TEST(tristate, matched_final) {
    constexpr auto z = matched_final{0};

    // >> skips right arg and returns reference to itself
    static_assert(&(z >> mismatch) == &z);
    static_assert(&(z >> regular_inc) == &z);
    static_assert(&(z >> final_inc) == &z);

    // commit_alt returns value (because z itself is temporary)
    static_assert(z.commit_alts() == z);
    static_assert(std::is_same_v< decltype(z.commit_alts()), matched_final<int> >);

    // commit_loop returns value (because z itself is temporary)
    static_assert(z.commit_loop() == z);
    static_assert(std::is_same_v< decltype(z.commit_loop()), matched_final<int> >);

    static_assert(z.rebind(""_ss) == matched_final{""_ss});
}

TEST(tristate, matched_final_halted) {
    constexpr auto z = matched_final_halted{0};

    // >> skips right arg and returns reference to itself
    static_assert(&(z >> mismatch) == &z);
    static_assert(&(z >> regular_inc) == &z);
    static_assert(&(z >> final_inc) == &z);

    // commit_alt returns value (because z itself is temporary)
    static_assert(z.commit_alts() == z);
    static_assert(std::is_same_v< decltype(z.commit_alts()), matched_final_halted<int> >);

    // commit_loop returns value (because z itself is temporary)
    static_assert(z.commit_loop() == z);
    static_assert(std::is_same_v< decltype(z.commit_loop()), matched_final_halted<int> >);

    static_assert(z.rebind(""_ss) == matched_final_halted{""_ss});
}

TEST(tristate, alternatives) {
    constexpr auto z = not_matched_yet{0};

    static_assert(&(z >> mismatch >> mismatch >> mismatch) == &z);
    static_assert(z >> mismatch >> regular_inc >> regular_inc == matched_regular{1});
    static_assert(z >> mismatch >> final_inc >> regular_inc == matched_final{10});
}

/// inplace

constexpr auto inplace_mismatch    = [](int& x) constexpr { return tristate_kind::not_matched_yet; };
constexpr auto inplace_regular_inc = [](int& x) constexpr { x += 1; return tristate_kind::matched_regular; };
constexpr auto inplace_final_inc   = [](int& x) constexpr { x += 10; return tristate_kind::matched_final; };

constexpr auto run_inplace(Inplace auto z, auto&& ... fs) {
    (z || ... || z.updated_by(fs));
    return z;
}
constexpr auto inplace_not_matched_yet(auto x) { return inplace_argument{x, tristate_kind::not_matched_yet}; }
constexpr auto inplace_matched_regular(auto x) { return inplace_argument{x, tristate_kind::matched_regular}; }
constexpr auto inplace_matched_final(auto x) { return inplace_argument{x, tristate_kind::matched_final}; }
constexpr auto inplace_matched_final_halted(auto x) { return inplace_argument{x, tristate_kind::matched_final_halted}; }
constexpr auto inplace_commit(auto x) { x.commit(); return x; }

TEST(tristate_inplace, not_matched_yet) {
    constexpr auto z = inplace_not_matched_yet(0);

    EXPECT_EQ(run_inplace(z, inplace_mismatch), inplace_not_matched_yet(0));
    EXPECT_EQ(run_inplace(z, inplace_regular_inc), inplace_matched_regular(1));
    EXPECT_EQ(run_inplace(z, inplace_final_inc), inplace_matched_final(10));

    EXPECT_EQ(inplace_commit(z), inplace_matched_final_halted(0));

    // EXPECT_EQ(z.rebind(""_ss) == not_matched_yet{""_ss});
}

TEST(tristate_inplace, matched_regular) {
    constexpr auto z = inplace_matched_regular(0);

    EXPECT_EQ(run_inplace(z, inplace_mismatch), z);
    EXPECT_EQ(run_inplace(z, inplace_regular_inc), z);
    EXPECT_EQ(run_inplace(z, inplace_final_inc), z);

    EXPECT_EQ(inplace_commit(z), inplace_not_matched_yet(0));

    // EXPECT_EQ(z.rebind(""_ss) == matched_regular{""_ss});
}

TEST(tristate_inplace, matched_final) {
    auto z = inplace_matched_final(0);

    EXPECT_EQ(run_inplace(z, inplace_mismatch), z);
    EXPECT_EQ(run_inplace(z, inplace_regular_inc), z);
    EXPECT_EQ(run_inplace(z, inplace_final_inc), z);

    EXPECT_EQ(inplace_commit(z), z);

    // EXPECT_EQ(z.rebind(""_ss) == matched_final{""_ss});
}

TEST(tristate_inplace, matched_final_halted) {
    auto z = inplace_matched_final_halted(0);

    EXPECT_EQ(run_inplace(z, inplace_mismatch), z);
    EXPECT_EQ(run_inplace(z, inplace_regular_inc), z);
    EXPECT_EQ(run_inplace(z, inplace_final_inc), z);

    EXPECT_EQ(inplace_commit(z), z);

    // EXPECT_EQ(z.rebind(""_ss) == matched_final_halted{""_ss});
}

TEST(tristate_inplace, alternatives) {
    constexpr auto z = inplace_not_matched_yet(0);

    EXPECT_EQ(run_inplace(z, inplace_mismatch, inplace_mismatch, inplace_mismatch), z);
    EXPECT_EQ(run_inplace(z, inplace_mismatch, inplace_regular_inc, inplace_regular_inc), inplace_matched_regular(1));
    EXPECT_EQ(run_inplace(z, inplace_mismatch, inplace_final_inc, inplace_regular_inc), inplace_matched_final(10));
}

} // namespace nn
