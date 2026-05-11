#include "nenormal/nenormal.h"
#include <gtest/gtest.h>

namespace examples::collatz_mini {

constexpr auto mul_continue = RULE("1[mul]",  "[mul]111111");
constexpr auto mul_end      = RULE("[mul]",   "");
constexpr auto div_continue = RULE("[div]11", "1[div]");
constexpr auto mul_start    = RULE("[div]1",  "[mul]1111");
constexpr auto div_end      = RULE("[div]",   "");
constexpr auto div_start    = RULE("11",      "1[div]");
constexpr auto stop   = FINAL_RULE("1",       "STOP");
constexpr auto zero   = FINAL_RULE("",        "ZERO"); // а вот нечего, ибо!

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

constexpr auto machine = MACHINE(program);

auto show_number = [](::nn::CtStr auto src) {
    // 111[mark]111
    ::std::string_view v = src.value.view();
    size_t n = v.size();
    size_t i = 0; while (v[i] == '1') ++i;
    size_t j = n; while (i < j && v[j-1] == '1') --j;

    ::std::ostringstream os;
    os << "<";
    if (i != 0) os << i;
    os << v.substr(i, j-i);
    if (j != n) os << (n-j);
    os << ">";

    return os.str();
};

auto trace = [](::nn::CtStr auto src, ::nn::SingleRule auto rule, ::nn::CtStr auto dst) {
    std::cout
        << ::std::setw(12) << ::std::left << show_number(src)
        << ::std::setw(22) << ::std::left << (src.value.view())
        << " => ("
        << ::std::setw(9) << ::std::quoted(rule.ct_search.value.view())
        << (rule.ct_state.value == ::nn::regular_state ? " ->  " : " ->. ")
        << ::std::setw(9) << ::std::quoted(rule.ct_replace.value.view())
        << ")"
        << " => "
        << ::std::setw(12) << ::std::left << show_number(dst)
        << ::std::setw(20) << ::std::left << (dst.value.view());

    constexpr auto d = dst.value.view();


    std::cout
        << ::std::endl;
};

auto run = [](::nn::CtStr auto src) {
    return machine(::nn::augmented_text{src, ::nn::side_effect{trace}}).text;
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
