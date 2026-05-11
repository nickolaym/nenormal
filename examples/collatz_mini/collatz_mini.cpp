#include "nenormal/nenormal.h"
#include <gtest/gtest.h>
#include <format>

namespace examples::collatz_mini {

// Elementary rules of the Collatz sequence program
// (listed in the order of appearance)

constexpr auto mul_continue = RULE("1[mul]",  "[mul]111111");
constexpr auto mul_end      = RULE("[mul]",   "");
constexpr auto div_continue = RULE("[div]11", "1[div]");
constexpr auto mul_start    = RULE("[div]1",  "[mul]1111");
constexpr auto div_end      = RULE("[div]",   "");
constexpr auto div_start    = RULE("11",      "1[div]");
constexpr auto stop   = FINAL_RULE("1",       "STOP");
constexpr auto zero   = FINAL_RULE("",        "ZERO"); // а вот нечего, ибо!

// Complete program

constexpr auto program = RULES(
    mul_continue,
    mul_end,
    div_continue,
    mul_start,
    div_end,
    div_start,
    stop,
    zero
);

// The executor of the program

constexpr auto machine = MACHINE(program);

// Parts of the callback that prints a table
// | step | source text | meaning of source | applied rule | destination text | meaning of destination |

// A text can take following forms:
// - 11...1{N times}                     - represents N
// - 11...1{L times}[div]11...1{R times} - while dividing by 2, N = L*2+R
// - 11...1{L times}[mul]11...1{R times} - while multiplying by 6, N = L*6+R
// - STOP and ZERO - final states
::std::string show_number(::std::string_view v) {
    // 111[mark]111
    size_t n = v.size();
    size_t i = 0; while (v[i] == '1') ++i;
    size_t j = n; while (i < j && v[j-1] == '1') --j;

    size_t left = i, right = (n-i);
    ::std::string_view tag = v.substr(i, j-i);

    if (tag == "")       return ::std::format("<{}>", left);
    if (tag == "[div]")  return ::std::format("<{}*2+{}>", left, right);
    if (tag == "[mul]")  return ::std::format("<{}*6+{}>", left, right);
    if (tag == "STOP" || tag == "ZERO") return ::std::format("<{}>", tag);
    return ::std::format("<{} {} {}>", left, tag, right); // unexpected
};

// A rule (regular or final)
::std::string show_rule(::nn::SingleRule auto rule) {
    constexpr auto s = rule.ct_search.value.view();
    constexpr auto r = rule.ct_replace.value.view();
    constexpr auto k = rule.ct_state.value;

    switch (k) {
    case ::nn::regular_state:
        return ::std::format("{} -> {}", s, r);
    case ::nn::final_state:
        return ::std::format("{} ->. {}", s, r);
    }
}

// Trace header shows markdown header of the table
void trace_header() {
    ::std::cout
        << ::std::format("|{:4}| {:21} | {:8} | {:21} | {:21} | {:8} |",
            "step",
            "src",
            "N =",
            "rule",
            "dst",
            "N' =")
        << ::std::endl
        << ::std::format("|{:-<4}|{:-<23}|{:-<10}|{:-<23}|{:-<23}|{:-<10}|",
            "",
            "",
            "",
            "",
            "",
            "")
        << ::std::endl;
}

// Trace line shows single step of the execution
void trace_line(
    int step,
    ::nn::CtStr auto src,
    ::nn::SingleRule auto rule,
    ::nn::CtStr auto dst
) {
    constexpr auto s = src.value.view();
    constexpr auto d = dst.value.view();

    ::std::cout
        << ::std::format("| {:2} | {:21} | {:8} | {:21} | {:21} | {:8} |",
            step,
            s,
            show_number(s),
            show_rule(rule),
            d,
            show_number(d))
        << ::std::endl;
};

// Trace footer shows the state after the last step
void trace_footer(::nn::CtStr auto dst, int step) {
    constexpr auto d = dst.value.view();
    ::std::cout
        << ::std::format("| {:2} | {:21} | {:8} | {:21} | {:21} | {:8} |",
            step,
            d,
            "",
            "",
            "",
            "")
        << ::std::endl;
}

// The callback
auto trace = [](
    int step,
    ::nn::CtStr auto src,
    ::nn::SingleRule auto rule,
    ::nn::CtStr auto dst
) -> int {
    trace_line(step, src, rule, dst);
    return step + 1;
};

// Run the executor with given source; return the result string
::nn::CtStr auto run(::nn::CtStr auto src) {
    trace_header();
    auto res = machine(::nn::augmented_text{src, ::nn::cumulative_effect{trace, 0}});
    trace_footer(res.text, res.aux.a);
    return res.text;
};

TEST(kollatz_mini, n_0) {
    EXPECT_EQ(run(CTSTR("")), CTSTR("ZERO"));
}

TEST(kollatz_mini, n_1) {
    EXPECT_EQ(run(CTSTR("1")), CTSTR("STOP"));
}

TEST(kollatz_mini, n_2) {
    EXPECT_EQ(run(CTSTR("11")), CTSTR("STOP"));
}

TEST(kollatz_mini, n_3) {
    EXPECT_EQ(run(CTSTR("111")), CTSTR("STOP"));
}

TEST(kollatz_mini, n_5) {
    EXPECT_EQ(run(CTSTR("11111")), CTSTR("STOP"));
}

} // namespace examples::collatz_mini
