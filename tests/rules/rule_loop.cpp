#include "nenormal/nenormal.h"
#include <gtest/gtest.h>
#include <format>
#include <cassert>

namespace test_rule_loop_ns {

// stess testing very long loops

template<size_t N>
constexpr ::nn::CtStr auto dots = ::nn::ct_chars(::nn::ct_size_v<N>, ::nn::ct_char_v<'.'>);

constexpr ::nn::CtStr auto zero = dots<0>;

constexpr ::nn::Rule auto single_decrement = RULE(".", "");
constexpr ::nn::Machine auto decrement = MACHINE(single_decrement);

template<size_t Limit>
constexpr ::nn::Rule auto limited_loop = ::nn::rule_loop<single_decrement, Limit>{};

template<size_t Limit>
constexpr ::nn::Machine auto limited_decrement = MACHINE_FROM_RULE(limited_loop<Limit>);

void print_step(int step, ::std::string_view o) {
    size_t n = o.size();
    size_t m =
        n > 1000 ? 1000:
        n > 100 ? 100 :
        n > 10 ? 10 :
        1;
    if (n % m == 0)
        ::std::cout << ::std::format("{:6} : {:6}", step, n) << ::std::endl;
}

constexpr auto count_steps =
[](int acc, ::nn::CtStr auto i, ::nn::SingleRule auto const& p, ::nn::CtStr auto o) {
    constexpr std::string_view ov = o.value.view();
    print_step(acc + 1, ov);
    return acc + 1;
};

auto run(::nn::CtStr auto s) {
    ::nn::Augmented auto input = ::nn::augmented_text{s, ::nn::cumulative_effect{0, count_steps}};
    ::nn::Augmented auto output = decrement(input);
    std::cout << "-----" << std::endl;
    return output.aux.a;
};

constexpr auto pure_run(::nn::CtStr auto s) {
    return decrement(s).value.size();
}


::std::string dots_str(size_t n) {
    return ::std::string(n, '.');
}

constexpr auto inplace_step = [](int acc, ::nn::SingleRule auto const& p, ::std::string const& t) {
    print_step(acc + 1, ::std::string_view(t));
    return acc + 1;
};

auto run_inplace(::std::string s) {
    ::nn::inplace_augmented_text arg{::std::move(s), ::nn::inplace_cumulative_effect{0, inplace_step}};
    return decrement(FWD(arg)).aux.a;
}

auto pure_run_inplace(::std::string s) {
    return decrement(FWD(s)).size();
}


TEST(rule_loop, pure) {
    static_assert(pure_run(dots<0>) == 0);
    static_assert(pure_run(dots<1>) == 0);
    static_assert(pure_run(dots<2>) == 0);
    static_assert(pure_run(dots<3>) == 0);
    static_assert(pure_run(dots<4>) == 0);
    static_assert(pure_run(dots<10>) == 0);
    static_assert(pure_run(dots<20>) == 0);
    // static_assert(pure_run(dots<100>) == 0);
    // static_assert(pure_run(dots<200>) == 0);
}

TEST(rule_loop, run) {
    EXPECT_EQ(run(dots<0>), 0);
    EXPECT_EQ(run(dots<1>), 1);
    EXPECT_EQ(run(dots<10>), 10);
    EXPECT_EQ(run(dots<20>), 20);
    EXPECT_EQ(run(dots<100>), 100);
}

TEST(rule_loop, pure_run_inplace) {
    EXPECT_EQ(pure_run_inplace(dots_str(0)), 0);
    EXPECT_EQ(pure_run_inplace(dots_str(1)), 0);
    EXPECT_EQ(pure_run_inplace(dots_str(10)), 0);
    EXPECT_EQ(pure_run_inplace(dots_str(20)), 0);
    EXPECT_EQ(pure_run_inplace(dots_str(100)), 0);
}

TEST(rule_loop, run_inplace) {
    EXPECT_EQ(run_inplace(dots_str(300)), 300);
}

TEST(rule_loop, limited_loops) {
    constexpr auto exam = [](::nn::CtSize auto loop_limit, ::nn::CtSize auto dots_len) constexpr {
        constexpr size_t L = loop_limit.value;
        constexpr size_t D = dots_len.value;

        constexpr ::nn::CtStr auto input_text = dots<D>;
        constexpr ::nn::RuleInput auto input = ::nn::not_matched_yet{input_text};
        constexpr ::nn::RuleOutput auto output = limited_loop<L>(input);

        if constexpr(L <= D) {
            static_assert(output == ::nn::not_matched_yet{dots<D - L>});
        } else {
            static_assert(output == ::nn::matched_final_halted{dots<0>});
        }
    };

    constexpr auto exam_loops = [=](::nn::CtSize auto dots_len) {
        // test unrolling without recursion
        exam(::nn::ct_size_v<0>, dots_len);
        exam(::nn::ct_size_v<1>, dots_len);
        exam(::nn::ct_size_v<2>, dots_len);
        exam(::nn::ct_size_v<3>, dots_len);
        exam(::nn::ct_size_v<4>, dots_len);
        exam(::nn::ct_size_v<5>, dots_len);
        exam(::nn::ct_size_v<6>, dots_len);
        exam(::nn::ct_size_v<7>, dots_len);
        exam(::nn::ct_size_v<8>, dots_len);
        exam(::nn::ct_size_v<9>, dots_len);
        exam(::nn::ct_size_v<10>, dots_len);
        // with single recursion
        exam(::nn::ct_size_v<11>, dots_len);
        exam(::nn::ct_size_v<15>, dots_len);
        exam(::nn::ct_size_v<16>, dots_len);
        // with double recursion
        exam(::nn::ct_size_v<25>, dots_len);
        exam(::nn::ct_size_v<26>, dots_len);
    };

    exam_loops(::nn::ct_size_v<0>);
    exam_loops(::nn::ct_size_v<5>);
    exam_loops(::nn::ct_size_v<15>);
    exam_loops(::nn::ct_size_v<25>);
}

TEST(rule_loop, inplace_limited_loop) {
    constexpr size_t L = 10; // there is no unrolling, so we test only one case
    constexpr auto machine = limited_decrement<L>;

    for (size_t D : {0, 9, 10, 11}) {
        size_t R = (D < L) ? 0 : (D - L);
        EXPECT_EQ(machine(dots_str(D)), dots_str(R));
    }
}

} // namespace test_rule_loop_ns


namespace test_extremal_loop_ns {

// decimal decrement of "<nnn>"

// start with "<nnn-->"" where nnn > 0
constexpr ::nn::Rule auto single_decrement = NAMED_RULE(single_decrement, RULES(
    RULE("<0", "<"), // cut leading zeros
    RULE("0--", "--9"), // subtract with borrow
    RULE("1--", "0"),
    RULE("2--", "1"),
    RULE("3--", "2"),
    RULE("4--", "3"),
    RULE("5--", "4"),
    RULE("6--", "5"),
    RULE("7--", "6"),
    RULE("8--", "7"),
    RULE("9--", "8")
));

// "<nnn>" -> "<nnn-->" --> "<mmm>" where mmm = nnn-1 --> etc until "<>" --> ""
constexpr ::nn::Rule auto repeated_decrement = RULES(
    RULE("<>", ""), // collapse implicit zero (and stop on empty string)
    single_decrement,
    RULE(">", "-->") // start decrement
);

TEST(decimal, single_step) {
    constexpr auto m = MACHINE(single_decrement);
    static_assert(m(CTSTR("<1-->")) == CTSTR("<>"));
    static_assert(m(CTSTR("<2-->")) == CTSTR("<1>"));
    static_assert(m(CTSTR("<3-->")) == CTSTR("<2>"));
    static_assert(m(CTSTR("<4-->")) == CTSTR("<3>"));
    static_assert(m(CTSTR("<5-->")) == CTSTR("<4>"));
    static_assert(m(CTSTR("<6-->")) == CTSTR("<5>"));
    static_assert(m(CTSTR("<7-->")) == CTSTR("<6>"));
    static_assert(m(CTSTR("<8-->")) == CTSTR("<7>"));
    static_assert(m(CTSTR("<9-->")) == CTSTR("<8>"));
    static_assert(m(CTSTR("<10-->")) == CTSTR("<9>"));
    static_assert(m(CTSTR("<20-->")) == CTSTR("<19>"));
    static_assert(m(CTSTR("<1000-->")) == CTSTR("<999>"));
}

TEST(decimal, repeated) {
    constexpr auto m = MACHINE(repeated_decrement);
    static_assert(m(CTSTR("<>")) == CTSTR(""));
    static_assert(m(CTSTR("<1>")) == CTSTR(""));
    static_assert(m(CTSTR("<9>")) == CTSTR(""));
    static_assert(m(CTSTR("<100>")) == CTSTR(""));
    static_assert(m(CTSTR("<300>")) == CTSTR(""));
}

TEST(decimal, count_iterations) {
    constexpr auto m = MACHINE(repeated_decrement);
    constexpr auto m500 = MACHINE_FROM_RULE((::nn::rule_loop<repeated_decrement, 500>{}));

    constexpr auto count = [=](auto machine, ::nn::CtStr auto s) {
        constexpr auto counter = [](int i, auto&&...) { return i+1; };
        constexpr auto input = ::nn::augmented_text{s, ::nn::cumulative_effect{0, counter}};
        constexpr auto output = machine(input);
        return std::pair{output.text, output.aux.a};
    };

    auto show_count = [=](auto machine, ::nn::CtStr auto s) {
        auto [t,c] = count(machine, s);
        std::cout
            << std::format("input {} runs {} steps stops {}",
                s.value.view(), c, t.value.view())
            << std::endl;
        return t.value.view().empty();
    };
    EXPECT_TRUE(show_count(m, CTSTR("<>")));
    EXPECT_TRUE(show_count(m, CTSTR("<1>")));
    EXPECT_TRUE(show_count(m, CTSTR("<10>")));
    EXPECT_TRUE(show_count(m, CTSTR("<100>")));
    EXPECT_TRUE(show_count(m, CTSTR("<300>")));
    EXPECT_TRUE(show_count(m, CTSTR("<100><100><100>")));
    // EXPECT_TRUE(show_count(m, CTSTR("<1000>"))); // it works, but compiles 2+ minutes
    // EXPECT_FALSE(show_count(m, CTSTR("<2500>"))); // it works, but compiles 3+ minutes

    EXPECT_TRUE(show_count(m500, CTSTR("<>")));
    EXPECT_TRUE(show_count(m500, CTSTR("<1>")));
    EXPECT_TRUE(show_count(m500, CTSTR("<10>")));
    EXPECT_TRUE(show_count(m500, CTSTR("<100>")));
    EXPECT_FALSE(show_count(m500, CTSTR("<300>")));
    EXPECT_FALSE(show_count(m500, CTSTR("<100><100><100><100><100>")));
}

} // namespace test_extremal_loop_ns
