#include "nenormal/nenormal.h"
#include <gtest/gtest.h>
#include <format>
#include <array>
#include <memory>

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

    void copy() { std::cout << "c"; ++copies; }
    void move() { std::cout << "m"; ++moves; }
    void init() { std::cout << "i"; ++inits; }

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
            debug_augmentation{
                cumulative_effect{
                    ctor_tracker_arg{tracker},
                    inc_tracker,
                },
                // note that the debug callback is copy-constructible
                // because we call it twice, - second time after the source is moved
                [tabptr = std::make_shared<int>(0)](::std::string_view msg) {
                    if (!tabptr) throw std::runtime_error("wtf");
                    int& tab = *tabptr;
                    if (msg.contains("body")) {
                        ::std::cout << msg;
                    }
                    else if (msg.starts_with("</")) {
                        --tab;
                        ::std::cout
                            << ::std::endl
                            << ::std::string(tab, ' ')
                            << msg
                            << ::std::endl
                            << ::std::string(tab, ' ');
                    }
                    else if (msg.starts_with("<")) {
                        ::std::cout
                            << ::std::endl
                            << ::std::string(tab, ' ')
                            << msg
                            << ::std::endl
                            << ::std::string(tab+1, ' ');
                        ++tab;
                    }
                    else
                        ::std::cout << msg;
                }
            }
        }
    };
}

auto examine_rule_impl(const char* title, CtStr auto s, Rule auto p, int expected_moves) {
    auto tracker = ctor_tracker{};
    std::cout << std::endl << title << std::endl;
    auto steps = p(make_input(&tracker, s)).value.aux.basic_aux.a.s;
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
constexpr Rule auto p_final = FINAL_RULE(".", "");

// catalogue of inputs
template<size_t N> constexpr auto ct_repa = ct_chars(ct_size_v<N>, ct_char_v<'a'>);

constexpr auto ct_a = ct_repa<1>;
constexpr auto ct_aaa = ct_repa<3>;
constexpr auto ct_a10 = ct_repa<10>;

constexpr auto ct_dot = CTSTR(".");

TEST(ctors, mismatch_rules) {
    // if there were no matched rules, no move ctors
    EXAMINE_RULE(ct_a, p_miss, 0);
    // depth does not matter
    EXAMINE_RULE(ct_a, RULES(RULES(RULES(p_miss))), 0);
}

TEST(ctors, single_match) {
    // 1 match - 1 init ctor, 0 moves directly from the rule
    EXAMINE_RULE(ct_a, p_match, 0);
    // +n levels of depth - +n moves
    EXAMINE_RULE(ct_a, RULES(p_match), 1);
    EXAMINE_RULE(ct_a, RULES(RULES(p_match)), 2);
    EXAMINE_RULE(ct_a, RULES(RULES(RULES(p_match))), 3);
    // mismatches before/after do not affect, only the level
    EXAMINE_RULE(ct_a, RULES(p_miss, p_match), 1);
    EXAMINE_RULE(ct_a, RULES(p_match, p_miss), 1);
    EXAMINE_RULE(ct_a, RULES(p_miss, p_match, p_miss), 1);
}

#define LOOP_BODY(p) (::nn::rule_loop_helpers_ns::rule_loop_body<(p)>{})

TEST(ctors, loop_body) {
    // result (by rvalue-ref) .commit_loop does +1 move
    // rule_loop_body does copy elision
    // so it just adds +1 move anyway
    EXAMINE_RULE(ct_a, LOOP_BODY(p_miss), 1);
    EXAMINE_RULE(ct_a, LOOP_BODY(p_match), 1);
    EXAMINE_RULE(ct_dot, LOOP_BODY(p_final), 1);
    // +n levels, as well
    EXAMINE_RULE(ct_a, LOOP_BODY(RULES(RULES(RULES(p_miss)))), 0 + 1);
    EXAMINE_RULE(ct_a, LOOP_BODY(RULES(RULES(RULES(p_match)))), 3 + 1);
    EXAMINE_RULE(ct_dot, LOOP_BODY(RULES(RULES(RULES(p_final)))), 3 + 1);
}

#define MULTIPLY_BODY(p, n) (::nn::rule_loop_helpers_ns::multiply_body<LOOP_BODY(p), (n)>{})

TEST(ctors, multiply_body_1) {
    // multiply 1 - copy elision from rule_loop_body, so +1 from it
    EXAMINE_RULE(ct_a, MULTIPLY_BODY(p_miss, 1), 1);
    EXAMINE_RULE(ct_a, MULTIPLY_BODY(p_match, 1), 1);
    EXAMINE_RULE(ct_dot, MULTIPLY_BODY(p_final, 1), 1);
}
TEST(ctors, multiply_body_2) {
    // multiply 2

    // body #1 return halted, +1
    // body #2 skipped (rvalue reference)
    // multiply forwards to rvalue, +1
    EXAMINE_RULE(ct_a, MULTIPLY_BODY(p_miss, 2), 2);

    // body #1 return not_matched_yet (from regular), +1
    // body #2 return halted, +1
    // multiply does copy elision
    EXAMINE_RULE(ct_a, MULTIPLY_BODY(p_match, 2), 2);
    // #1 return not_matched_yet, +1
    // #2 return not_matched_yet, +1
    // copy elision
    EXAMINE_RULE(ct_aaa, MULTIPLY_BODY(p_match, 2), 2);

    // body #1 return final, +1
    // body #2 skipped
    // multiply forwards to rvalue, +1
    EXAMINE_RULE(ct_dot, MULTIPLY_BODY(p_final, 2), 2);
}
TEST(ctors, multiply_body_3) {
    // multiply 3

    // body #1 return halted, +1
    // body #2 and #3 skipped
    // multiply forwards to value, +1
    EXAMINE_RULE(ct_a, MULTIPLY_BODY(p_miss, 3), 2);

    // body #1 return not_matched_yet (from regular), +1
    // body #2 return halted, +1
    // body #3 skipped
    // multiply forwards to value, +1
    EXAMINE_RULE(ct_a, MULTIPLY_BODY(p_match, 3), 3);
    // #1 nmy +1; #2 nmy +1; #3 nmy +1; copy elision
    EXAMINE_RULE(ct_aaa, MULTIPLY_BODY(p_match, 3), 3);

    // body #1 return final, +1
    // body #2 and #3 skipped
    // multiply forwards to value, +1
    EXAMINE_RULE(ct_dot, MULTIPLY_BODY(p_final, 3), 2);
}
TEST(ctors, multiply_body_10) {
    // multiply 10

    // body #1 return halted, +1
    // rest bodies skipped
    // multiply forwards to value, +1
    EXAMINE_RULE(ct_a, MULTIPLY_BODY(p_miss, 10), 2);

    // body #1 return not_matched_yet (from regular), +1
    // body #2 return halted, +1
    // rest bodies skipped
    // multiply forwards to value, +1
    EXAMINE_RULE(ct_a, MULTIPLY_BODY(p_match, 10), 3);
    // #1..#3 nmy, +3; #4 halted, +1; rest skipped; by value +1
    EXAMINE_RULE(ct_aaa, MULTIPLY_BODY(p_match, 10), 5);
    // #1..#10 nmy, +10; copy elision
    EXAMINE_RULE(ct_a10, MULTIPLY_BODY(p_match, 10), 10);

    // body #1 return final, +1
    // rest bodies skipped
    // multiply forwards to value, +1
    EXAMINE_RULE(ct_dot, MULTIPLY_BODY(p_final, 10), 2);
}

// test body with logarithmic depth
// - unroll 10
// - multiplication 2
#define REPEAT_BODY_10_2(p, limit) \
    (::nn::rule_loop_helpers_ns::repeat_body<LOOP_BODY(p), (limit), 10, 2>{})

TEST(ctors, repeat_body_10_2_limit_0) {
    // repeat 0 - does nothing, return by value, +1
    EXAMINE_RULE(ct_aaa, REPEAT_BODY_10_2(p_miss, 0), 1);
    EXAMINE_RULE(ct_aaa, REPEAT_BODY_10_2(p_match, 0), 1);
    EXAMINE_RULE(ct_dot, REPEAT_BODY_10_2(p_final, 0), 1);
}

TEST(ctors, repeat_body_10_2_limit_1_to_10) {
    // repeat 1..10 calls multiply and does copy elision

    // 1:
    EXAMINE_RULE(ct_a, REPEAT_BODY_10_2(p_miss, 1), 1);
    EXAMINE_RULE(ct_a, REPEAT_BODY_10_2(p_match, 1), 1);
    EXAMINE_RULE(ct_dot, REPEAT_BODY_10_2(p_final, 1), 1);

    // 2:
    EXAMINE_RULE(ct_a, REPEAT_BODY_10_2(p_miss, 2), 2);
    EXAMINE_RULE(ct_a, REPEAT_BODY_10_2(p_match, 2), 2);
    EXAMINE_RULE(ct_aaa, REPEAT_BODY_10_2(p_match, 2), 2);
    EXAMINE_RULE(ct_dot, REPEAT_BODY_10_2(p_final, 2), 2);

    // 3:
    EXAMINE_RULE(ct_a, REPEAT_BODY_10_2(p_miss, 3), 2);
    EXAMINE_RULE(ct_a, REPEAT_BODY_10_2(p_match, 3), 3);
    EXAMINE_RULE(ct_aaa, REPEAT_BODY_10_2(p_match, 3), 3);
    EXAMINE_RULE(ct_dot, REPEAT_BODY_10_2(p_final, 3), 2);
    // 10:
    EXAMINE_RULE(ct_a, REPEAT_BODY_10_2(p_miss, 10), 2);
    EXAMINE_RULE(ct_a, REPEAT_BODY_10_2(p_match, 10), 3);
    EXAMINE_RULE(ct_aaa, REPEAT_BODY_10_2(p_match, 10), 5);
    EXAMINE_RULE(ct_a10, REPEAT_BODY_10_2(p_match, 10), 10);
    EXAMINE_RULE(ct_dot, REPEAT_BODY_10_2(p_final, 10), 2);
}

// repeat<body,10> = (multiply<body,10>) + repeat<multiply<body,2>,5>
constexpr auto rep20 = REPEAT_BODY_10_2(p_match, 20);

TEST(ctors, repeat_body_10_2_limit_20_first_part) {
    // halted inside first multiply<body,10>...
    EXAMINE_RULE(ct_repa<0>, rep20, 3); // mult10(halted +1, skip, return +1), skip, return +1
    EXAMINE_RULE(ct_repa<1>, rep20, 4); // mult10(nmy +1, halted +1, skip, return +1), skip, return +1
    EXAMINE_RULE(ct_repa<2>, rep20, 5); // mult10(nmy*2 = +2, halted +1, skip, return +1), skip, return +1
    // etc up to 8
    EXAMINE_RULE(ct_repa<8>, rep20, 11); // mult10(nmy*8 = +8, halted +1, skip, return +1), skip, return +1
    // 9
    EXAMINE_RULE(ct_repa<9>, rep20, 11); // mult10(nmy*9 = +9, halted +1, copy elision), skip, return +1
}
TEST(ctors, repeat_body_10_2_limit_20_second_part) {
    // halted inside second repeat<multiply<body,2>>...
    EXAMINE_RULE(ct_repa<10>, rep20, 13); // mult10(nmy*10 = +10), rep5(mult2(halted +1, ret +1), ret +1)
    EXAMINE_RULE(ct_repa<11>, rep20, 13); // mult10(nmy*10 = +10), rep5(mult2(nmy +1, halted +1), ret +1)

    EXAMINE_RULE(ct_repa<12>, rep20, 15); // mult10(nmy*10 = +10), rep5(mult2(+2)*2, ret +1)
    EXAMINE_RULE(ct_repa<13>, rep20, 15); // mult10(nmy*10 = +10), rep5(mult2(+2)*2, ret +1)

    EXAMINE_RULE(ct_repa<14>, rep20, 17); // mult10(nmy*10 = +10), rep5(mult2(+2)*3, ret +1)
    EXAMINE_RULE(ct_repa<15>, rep20, 17); // mult10(nmy*10 = +10), rep5(mult2(+2)*3, ret +1)

    EXAMINE_RULE(ct_repa<16>, rep20, 19); // mult10(nmy*10 = +10), rep5(mult2(+2)*4, ret +1)
    EXAMINE_RULE(ct_repa<17>, rep20, 19); // mult10(nmy*10 = +10), rep5(mult2(+2)*4, ret +1)

    EXAMINE_RULE(ct_repa<18>, rep20, 20); // mult10(nmy*10 = +10), rep5(mult2(+2)*5)
    EXAMINE_RULE(ct_repa<19>, rep20, 20); // mult10(nmy*10 = +10), rep5(mult2(+2)*5)
    // not halted yet
    EXAMINE_RULE(ct_repa<20>, rep20, 20); // mult10(nmy*10 = +10), rep5(mult2(+2)*5)
}

// repeat<body,50> =
//   multiply<body,10> +
//   repeat<multiply<body,2>,20> =
//     multiply<multiply<body,2>,10> +
//       multiply<multiply<multiply<body,2>,2>,5>
constexpr auto rep50 = REPEAT_BODY_10_2(p_match, 50);

TEST(ctors, repeat_body_10_2_limit_50_first_part) {
    // first part (0..9) behaves as above
    EXAMINE_RULE(ct_repa<0>, rep50, 3);
    EXAMINE_RULE(ct_repa<9>, rep50, 11);
}
TEST(ctors, repeat_body_10_2_limit_50_second_part) {
    // second part behaves as above, +1 ret
    // 10 + (0..19)
    EXAMINE_RULE(ct_repa<10>, rep50, 14); // 10 + 2*1 + ret 1 + ret 1
    EXAMINE_RULE(ct_repa<17>, rep50, 20); // 10 + 2*4 + ret 1 + ret 1
    EXAMINE_RULE(ct_repa<29>, rep50, 31); // 10 + 2*10 + elision + ret 1
}
TEST(ctors, repeat_body_10_2_limit_50_third_part) {
    // third part (30..50)
    // mul2(mul2) works as follows:
    // 1: (halt,ret),ret = 3
    // 2: (nmy,halt),ret = 3
    // 3: (nmy,nmy),(halt,ret) = 4
    // 4: (nmy,nmy),(nmy,halt) = 4
    EXAMINE_RULE(ct_repa<30>, rep50, 34); // 10 + 2*10 + mul4{+3}*1 + ret 1
    EXAMINE_RULE(ct_repa<31>, rep50, 34); // 10 + 2*10 + mul4{+3}*1 + ret 1
    EXAMINE_RULE(ct_repa<32>, rep50, 35); // 10 + 2*10 + mul4{+4}*1 + ret 1
    EXAMINE_RULE(ct_repa<33>, rep50, 35); // 10 + 2*10 + mul4{+4}*1 + ret 1
    // etc up to 45
    EXAMINE_RULE(ct_repa<44>, rep50, 47); // 10 + 2*10 + mul4{+4}*4 + ret 1
    EXAMINE_RULE(ct_repa<45>, rep50, 47); // 10 + 2*10 + mul4{+4}*4 + ret 1
    // last 4 - copy elision
    EXAMINE_RULE(ct_repa<46>, rep50, 49); // 10 + 2*10 + mul4{+4}*4 + mul4{+3}
    EXAMINE_RULE(ct_repa<47>, rep50, 49); // 10 + 2*10 + mul4{+4}*4 + mul4{+3}
    EXAMINE_RULE(ct_repa<48>, rep50, 50); // 10 + 2*10 + mul4{+4}*4 + mul4{+4}
    EXAMINE_RULE(ct_repa<49>, rep50, 50); // 10 + 2*10 + mul4{+4}*4 + mul4{+4}
    // 50 - returns nmy
    EXAMINE_RULE(ct_repa<50>, rep50, 50); // 10 + 2*10 + mul4{+4}*4 + mul4{+4}
}

// rule loop runs repeat<5000, 50, 1> and does copy elision

constexpr auto loop = RULE_LOOP(p_match);

TEST(ctors, loop) {
    EXAMINE_RULE(ct_repa<0>, loop, 3);
    EXAMINE_RULE(ct_repa<9>, loop, 12);

    EXAMINE_RULE(ct_repa<10>, loop, 13);
    EXAMINE_RULE(ct_repa<29>, loop, 32);

    EXAMINE_RULE(ct_repa<30>, loop, 33);
    EXAMINE_RULE(ct_repa<49>, loop, 51);
}

////////

struct move_counter {
    size_t moves = 0;
    constexpr move_counter(size_t m) : moves(m) {}
    constexpr move_counter(move_counter&& src) : moves(src.moves + 1) {}
};

constexpr auto run_counting_moves(CtStr auto s, Rule auto p) {
    return p(not_matched_yet{augmented_text{s, cumulative_effect{
        move_counter{0},
        [](move_counter const& mc, auto...) { return move_counter(mc.moves); },
    }}}).value.aux.a.moves;
}

// check that run_counting_moves works
// static_assert(run_counting_moves(ct_a, p_miss) == 0);

template<size_t... ns>
constexpr auto steps_and_moves =
    std::array<std::pair<size_t, size_t>, (sizeof...(ns))>{
        std::pair{ns, run_counting_moves(ct_repa<ns>, loop)} ...
    };

TEST(moves, count) {
    // 10n iterations make
    // - 10n moves with applying regular rule
    // - 1 move with halting
    // - 1 move with return by value from the middle of multiply_body
    // - 1 move with return by value instead of recursion
    // totally, 10n + 3
    auto table10 = steps_and_moves<
        0, 10, 20, 30, 40,
        50, 60, 70, 80, 90,
        100, 200, 300, 400, 500>;
    for (auto [num_steps, num_move_ctors] : table10) {
        EXPECT_EQ(num_steps + 3, num_move_ctors);
    }

    // 50n-1 iterations make
    // - 50n-1 moves with applying regular rule
    // - 1 move with halting
    // copy elision from the end of multiply_body
    // - 1 move with return by value instead of recursion
    // totally, 50n-1 + 2
    auto table49 = steps_and_moves<49, 99, 149, 199>;
    for (auto [num_steps, num_move_ctors] : table49) {
        EXPECT_EQ(num_steps + 2, num_move_ctors);
    }
}

} // namespace
} // namespace nn
