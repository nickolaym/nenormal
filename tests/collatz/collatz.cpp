#include "./program.h"
#include <gtest/gtest.h>

TEST(collatz, stop_cases) {
    static_assert(machine(CTSTR("")) == CTSTR(""));
    static_assert(machine(CTSTR("<1>")) == CTSTR(""));
}

TEST(collatz, case_2) {
    static_assert(machine(CTSTR("<11>")) == CTSTR("2"));
}

TEST(collatz, case_3) {
    constexpr auto step = [](CtStr auto s) { return program(s).data; };

    // 3 -> 3x+1 = 10
    constexpr auto t0 = CTSTR("<111>");
    constexpr auto t1 = step(t0); static_assert(t1 == CTSTR("<:11c1>"));
    constexpr auto t2 = step(t1); static_assert(t2 == CTSTR("<:11o1111>3"));
    constexpr auto t3 = step(t2); static_assert(t3 == CTSTR("<:1o1111111>3"));
    constexpr auto t4 = step(t3); static_assert(t4 == CTSTR("<:o1111111111>3"));
    constexpr auto t5 = step(t4); static_assert(t5 == CTSTR("<1111111111>3"));

    // whole run: 3 -> 10 -> 5 -> 16 -> 8 -> 4 -> 2 -> 1
    // 2222323
    constexpr auto tf = machine(t0);
    static_assert(tf == CTSTR("2222323"));
}

TEST(collatz, runtime) {
    auto print_text = [](CtStr auto t) {
        std::string_view v = t.value.view();
        if (v.size() > 1 && v[1] == ':') {
            std::cout << "  ";
        } else {
            std::cout << "- ";
        }
        std::cout << v << std::endl;
    };
    auto trace = [&](CtStr auto src, SingleRule auto p, CtStr auto dst) {
        print_text(dst);
    };
    auto show = [&](CtStr auto src) {
        print_text(src);
        auto dst = machine(augmented_text{src, side_effect{trace}}).text;
        std::string_view v = dst.value.view();
        std::cout << v.size() << " iterations" << std::endl;
        std::cout << std::endl;
    };

    show(CTSTR("<1>"));
    show(CTSTR("<11>"));
    show(CTSTR("<111>"));
    show(CTSTR("<1111111>")); // 7 - 22 - 11 - 34 - 17 - 52 - 13 - 40 - 20 - 10 - 5 - 16 - 8 - 4 - 2 - 1
}

