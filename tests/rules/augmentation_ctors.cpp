#include "nenormal/nenormal.h"
#include <gtest/gtest.h>

// Augmentation should call as few copy/move constructors as possible.
// If the input is rvalue, only moves are expected.

namespace nn {
namespace {

// Shared counter of ctors of a family of objects

struct ctor_tracker {
    int copies = 0;
    int moves = 0;
    int inits = 0;

    int total() const { return copies + moves + inits; }

    void copy() { nl(); std::cout << "c"; ++copies; }
    void move() { nl(); std::cout << "m"; ++moves; }
    void init() { nl(); std::cout << "i"; ++inits; }
    void nl() { int t = total(); if (t != 0 && (t % 10 == 0)) std::cout << "\n    "; }

    friend std::ostream& operator << (std::ostream& os, ctor_tracker const& t) {
        return os << "tracker{c=" << t.copies << ", m=" << t.moves << ", i=" << t.inits << "}";
    }
};

// A tracked object - an argument of cumulative augmentation effect

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
};

auto inc_tracker = [](ctor_tracker_arg const& a, auto...) {
    // creates new instance, not copy/move of the source arg
    return ctor_tracker_arg{a.t, a.s + 1};
};

auto make_input(ctor_tracker* tracker, CtStr auto s) {
    return not_matched_yet{
        augmented_text{
            s,
            cumulative_effect{
                inc_tracker,
                ctor_tracker_arg{tracker}
            }
        }
    };
}

auto examine_rule_impl(const char* title, CtStr auto s, Rule auto p, int expected_moves) {
    auto tracker = ctor_tracker{};
    std::cout << std::endl << title << std::endl;
    auto steps = p(make_input(&tracker, s)).value.aux.a.s;
    std::cout << std::endl << "    " << tracker << std::endl;
    EXPECT_EQ(tracker.copies, 0);
    EXPECT_EQ(tracker.moves, expected_moves);
    EXPECT_EQ(tracker.inits, steps);
}

auto examine_machine_impl(const char* title, CtStr auto s, Machine auto m, int expected_moves) {
    auto tracker = ctor_tracker{};
    std::cout << std::endl << title << std::endl;
    auto steps = m(make_input(&tracker, s).value).aux.a.s;
    std::cout << std::endl << "    " << tracker << std::endl;
    EXPECT_EQ(tracker.copies, 0);
    EXPECT_EQ(tracker.moves, expected_moves);
    EXPECT_EQ(tracker.inits, steps);
}

#define EXAMINE_RULE(s, p, em) ([]{ \
    SCOPED_TRACE("examine rule " #s " >> " #p); \
    examine_rule_impl(#s " >> " #p, s, p, em); \
}())

#define EXAMINE_MACHINE(s, m, em) ([]{ \
    SCOPED_TRACE("examine machine " #s " >> " #m); \
    examine_machine_impl(#s " >> " #m, s, m, em); \
}())

// catalogue of rules
constexpr Rule auto p_miss  = RULE("x", "");
constexpr Rule auto p_match = RULE("a", "");
constexpr Rule auto p_final = RULE(".", "");

TEST(ctors, mismatch_rules) {
    // if there were no matched rules, no move ctors
    EXAMINE_RULE(CTSTR("a"), p_miss, 0);
    // depth does not matter
    EXAMINE_RULE(CTSTR("a"), RULES(RULES(RULES(p_miss))), 0);
}

TEST(ctors, single_match) {
    // 1 match - 1 init ctor, 0 moves directly from the rule
    EXAMINE_RULE(CTSTR("a"), p_match, 0);
    // +n levels of depth - +n moves
    EXAMINE_RULE(CTSTR("a"), RULES(p_match), 1);
    EXAMINE_RULE(CTSTR("a"), RULES(RULES(p_match)), 2);
    EXAMINE_RULE(CTSTR("a"), RULES(RULES(RULES(p_match))), 3);
    // mismatches before/after do not affect, only the level
    EXAMINE_RULE(CTSTR("a"), RULES(p_miss, p_match), 1);
    EXAMINE_RULE(CTSTR("a"), RULES(p_match, p_miss), 1);
    EXAMINE_RULE(CTSTR("a"), RULES(p_miss, p_match, p_miss), 1);
}

#define LOOP_BODY(p) (::nn::rule_loop_helpers_ns::rule_loop_body<(p)>{})

TEST(ctors, loop_body) {
    // result (by rvalue-ref) .commit_loop does +1 move
    // rule_loop_body does copy elision
    // so it just adds +1 move anyway
    EXAMINE_RULE(CTSTR("a"), LOOP_BODY(p_miss), 1);
    EXAMINE_RULE(CTSTR("a"), LOOP_BODY(p_match), 1);
    EXAMINE_RULE(CTSTR("."), LOOP_BODY(p_final), 1);
    // +n levels, as well
    EXAMINE_RULE(CTSTR("a"), LOOP_BODY(RULES(RULES(RULES(p_miss)))), 0 + 1);
    EXAMINE_RULE(CTSTR("a"), LOOP_BODY(RULES(RULES(RULES(p_match)))), 3 + 1);
    EXAMINE_RULE(CTSTR("."), LOOP_BODY(RULES(RULES(RULES(p_final)))), 3 + 1);
}

#define REPEAT_LOOP_BODY(p, n) (::nn::rule_loop_helpers_ns::repeat_body<LOOP_BODY(p), (n)>{})

TEST(ctors, repeat_body_0) {
    // repeat ultimatively returns rvalue, so +1 itself
    // repeat 0 = no body
    EXAMINE_RULE(CTSTR("a"), REPEAT_LOOP_BODY(p_miss, 0), 1);
    EXAMINE_RULE(CTSTR("a"), REPEAT_LOOP_BODY(p_match, 0), 1);
}
TEST(ctors, repeat_body_1) {
    // repeat 1 - copy elision from rule_loop_body, so +1 from it
    EXAMINE_RULE(CTSTR("a"), REPEAT_LOOP_BODY(p_miss, 1), 1);
    EXAMINE_RULE(CTSTR("a"), REPEAT_LOOP_BODY(p_match, 1), 1);
    EXAMINE_RULE(CTSTR("."), REPEAT_LOOP_BODY(p_final, 1), 1);
}
TEST(ctors, repeat_body_2) {
    // repeat 2

    // body #1 return halted, +1
    // body #2 skipped (rvalue reference)
    // repeat forwards to rvalue, +1
    EXAMINE_RULE(CTSTR("a"), REPEAT_LOOP_BODY(p_miss, 2), 2);

    // body #1 return not_matched_yet (from regular), +1
    // body #2 return halted, +1
    // repeat does copy elision
    EXAMINE_RULE(CTSTR("a"), REPEAT_LOOP_BODY(p_match, 2), 2);

    // body #1 return final, +1
    // body #2 skipped
    // repeat forwards to rvalue, +1
    EXAMINE_RULE(CTSTR("."), REPEAT_LOOP_BODY(p_final, 2), 2);
}
TEST(ctors, repeat_body_3) {
    // repeat 3

    // body #1 return halted, +1
    // body #2 and #3 skipped
    // repeat forwards to value, +1
    EXAMINE_RULE(CTSTR("a"), REPEAT_LOOP_BODY(p_miss, 3), 2);

    // body #1 return not_matched_yet (from regular), +1
    // body #2 return halted, +1
    // body #3 skipped
    // repeat forwards to value, +1
    EXAMINE_RULE(CTSTR("a"), REPEAT_LOOP_BODY(p_match, 3), 3);

    // body #1 return final, +1
    // body #2 and #3 skipped
    // repeat forwards to value, +1
    // ???
    EXAMINE_RULE(CTSTR("."), REPEAT_LOOP_BODY(p_final, 3), 3);
}
TEST(ctors, repeat_body_10) {
    // repeat 10

    // body #1 return halted, +1
    // rest bodies skipped
    // repeat forwards to value, +1
    EXAMINE_RULE(CTSTR("a"), REPEAT_LOOP_BODY(p_miss, 10), 2);

    // body #1 return not_matched_yet (from regular), +1
    // body #2 return halted, +1
    // rest bodies skipped
    // repeat forwards to value, +1
    EXAMINE_RULE(CTSTR("a"), REPEAT_LOOP_BODY(p_match, 10), 3);

    // body #1 return final, +1
    // rest bodies skipped
    // repeat forwards to value, +1
    // ???
    EXAMINE_RULE(CTSTR("."), REPEAT_LOOP_BODY(p_final, 10), 3);
}

} // namespace
} // namespace nn
