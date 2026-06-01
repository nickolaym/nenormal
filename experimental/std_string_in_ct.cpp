#include <string>
#include <vector>
#include <utility>
#include <array>
#include <iostream>

#include <gtest/gtest.h>

// #include "nenormal/str.h"

enum class kind_t { nmy, reg, fin };

using result_t = std::pair<std::string, kind_t>;


struct static_str {
    char buf[100] = {};
    size_t len = 0;
    constexpr static_str(const char* s) {
        while(*s) buf[len++] = *s++;
        buf[len] = 0;
    }
    constexpr const char* begin() const { return buf; }
    constexpr const char* end() const { return buf + len; }
    constexpr size_t size() const { return len; }
};


struct rule {
    static_str s, r;
    kind_t k;

    constexpr result_t operator()(const std::string& t) const {
        size_t i = t.find(s.buf);
        if (i == t.npos) return {{}, kind_t::nmy};
        else return {t.substr(0, i) + r.buf + t.substr(i + s.size()), k};
    }
};

template<size_t n>
struct rules {
    const std::array<rule, n> v;

    constexpr result_t operator()(const std::string& t) const {
        for (const rule& p : v) {
            result_t res = p(t);
            if (res.second != kind_t::nmy) return res;
        }
        return {t, kind_t::nmy};
    }
};

template<size_t n>
struct loop {
    rules<n> p;

    constexpr result_t operator()(std::string t) const {
        return (*this)(t, [](auto&&, auto&&) {});
    }
    constexpr result_t operator()(std::string t, auto cb) const {
        while (true) {
            result_t res = p(t);
            // std::cout << t << " -> " << res.first << std::endl;
            cb(t, res.first);
            if (res.second != kind_t::reg) return res;
            t = std::move(res.first);
        }
    }
};

constexpr auto sss = static_str{"a"};
constexpr auto rrr = rule{"a","b",kind_t::reg};
constexpr auto aaa = std::array{rrr};
constexpr auto lll = loop{rules{std::array{rrr}}};

constexpr auto mmm = [](std::string t) { return lll(t).first; };

static_assert(mmm("aaa") == "bbb");

#define MACHINE(...) \
    (loop{rules{std::array{ \
        __VA_ARGS__ \
    }}}) \
// endmacro
// #define MACHINE(...) \
// ([](std::string t, auto... extra) constexpr { \
//     constexpr auto prog = loop{rules{std::array{ \
//         __VA_ARGS__ \
//     }}}; \
//     return prog(t, extra...); \
// }) \
// // endmacro

#define RULE(s,r) rule{(s),(r),kind_t::reg}
#define FINAL_RULE(s,r) rule{(s),(r),kind_t::fin}

constexpr auto collatz = MACHINE(
    FINAL_RULE("^$", "zero"),
    FINAL_RULE("^.$", "one"),
    RULE("^..", "^:"), RULE(":..", "::"),
    RULE(":.$", ":6....$."), RULE(":6", "6......"), RULE("^6", "^"),
    RULE(":$", ":1$."), RULE(":1", "1."), RULE("^1", "^"),
);

TEST(collatz,runtime) {
    EXPECT_EQ(collatz("^.......$").first, "one................");
    static_assert(collatz("^.......$").first == "one................");
}

constexpr auto decimal = MACHINE(
    /////////////////////////
    // mult3

    // start from :>
    RULE("1:>", " *3+0 4>"),
    RULE("3:>", " *3+1 0>"),
    RULE("5:>", " *3+1 6>"),
    RULE("7:>", " *3+2 2>"),
    RULE("9:>", " *3+2 8>"),

    // multiply *3 with carry
    RULE("0 *3+0 ", " *3+0 0"),
    RULE("0 *3+1 ", " *3+0 1"),
    RULE("0 *3+2 ", " *3+0 2"),
    RULE("0 *3+3 ", " *3+0 3"),

    RULE("1 *3+0 ", " *3+0 3"),
    RULE("1 *3+1 ", " *3+0 4"),
    RULE("1 *3+2 ", " *3+0 5"),
    RULE("1 *3+3 ", " *3+0 6"),

    RULE("2 *3+0 ", " *3+0 6"),
    RULE("2 *3+1 ", " *3+0 7"),
    RULE("2 *3+2 ", " *3+0 8"),
    RULE("2 *3+3 ", " *3+0 9"),

    RULE("3 *3+0 ", " *3+0 9"),
    RULE("3 *3+1 ", " *3+1 0"),
    RULE("3 *3+2 ", " *3+1 1"),
    RULE("3 *3+3 ", " *3+1 2"),

    RULE("4 *3+0 ", " *3+1 2"),
    RULE("4 *3+1 ", " *3+1 3"),
    RULE("4 *3+2 ", " *3+1 4"),
    RULE("4 *3+3 ", " *3+1 5"),

    RULE("5 *3+0 ", " *3+1 5"),
    RULE("5 *3+1 ", " *3+1 6"),
    RULE("5 *3+2 ", " *3+1 7"),
    RULE("5 *3+3 ", " *3+1 8"),

    RULE("6 *3+0 ", " *3+1 8"),
    RULE("6 *3+1 ", " *3+1 9"),
    RULE("6 *3+2 ", " *3+2 0"),
    RULE("6 *3+3 ", " *3+2 1"),

    RULE("7 *3+0 ", " *3+2 1"),
    RULE("7 *3+1 ", " *3+2 2"),
    RULE("7 *3+2 ", " *3+2 3"),
    RULE("7 *3+3 ", " *3+2 4"),

    RULE("8 *3+0 ", " *3+2 4"),
    RULE("8 *3+1 ", " *3+2 5"),
    RULE("8 *3+2 ", " *3+2 6"),
    RULE("8 *3+3 ", " *3+2 7"),

    RULE("9 *3+0 ", " *3+2 7"),
    RULE("9 *3+1 ", " *3+2 8"),
    RULE("9 *3+2 ", " *3+2 9"),
    RULE("9 *3+3 ", " *3+3 0"),

    // finish at < (implicit leading 0)
    RULE("< *3+0 ", " +1 <"),
    RULE("< *3+1 ", " +1 <1"),
    RULE("< *3+2 ", " +1 <2"),
    RULE("< *3+3 ", " +1 <3"),

    /////////////////////////
    // div2

    // start from :>
    RULE("0:>", " *5+0 >"),
    RULE("2:>", " *5+1 >"),
    RULE("4:>", " *5+2 >"),
    RULE("6:>", " *5+3 >"),
    RULE("8:>", " *5+4 >"),

    // multiply *5 with carry
    RULE("0 *5+0 ", " *5+0 0"),
    RULE("0 *5+1 ", " *5+0 1"),
    RULE("0 *5+2 ", " *5+0 2"),
    RULE("0 *5+3 ", " *5+0 3"),
    RULE("0 *5+4 ", " *5+0 4"),
    RULE("0 *5+5 ", " *5+0 5"),

    RULE("1 *5+0 ", " *5+0 5"),
    RULE("1 *5+1 ", " *5+0 6"),
    RULE("1 *5+2 ", " *5+0 7"),
    RULE("1 *5+3 ", " *5+0 8"),
    RULE("1 *5+4 ", " *5+0 9"),
    RULE("1 *5+5 ", " *5+1 0"),

    RULE("2 *5+0 ", " *5+1 0"),
    RULE("2 *5+1 ", " *5+1 1"),
    RULE("2 *5+2 ", " *5+1 2"),
    RULE("2 *5+3 ", " *5+1 3"),
    RULE("2 *5+4 ", " *5+1 4"),
    RULE("2 *5+5 ", " *5+1 5"),

    RULE("3 *5+0 ", " *5+1 5"),
    RULE("3 *5+1 ", " *5+1 6"),
    RULE("3 *5+2 ", " *5+1 7"),
    RULE("3 *5+3 ", " *5+1 8"),
    RULE("3 *5+4 ", " *5+1 9"),
    RULE("3 *5+5 ", " *5+2 0"),

    RULE("4 *5+0 ", " *5+2 0"),
    RULE("4 *5+1 ", " *5+2 1"),
    RULE("4 *5+2 ", " *5+2 2"),
    RULE("4 *5+3 ", " *5+2 3"),
    RULE("4 *5+4 ", " *5+2 4"),
    RULE("4 *5+5 ", " *5+2 5"),

    RULE("5 *5+0 ", " *5+2 5"),
    RULE("5 *5+1 ", " *5+2 6"),
    RULE("5 *5+2 ", " *5+2 7"),
    RULE("5 *5+3 ", " *5+2 8"),
    RULE("5 *5+4 ", " *5+2 9"),
    RULE("5 *5+5 ", " *5+3 0"),

    RULE("6 *5+0 ", " *5+3 0"),
    RULE("6 *5+1 ", " *5+3 1"),
    RULE("6 *5+2 ", " *5+3 2"),
    RULE("6 *5+3 ", " *5+3 3"),
    RULE("6 *5+4 ", " *5+3 4"),
    RULE("6 *5+5 ", " *5+3 5"),

    RULE("7 *5+0 ", " *5+3 5"),
    RULE("7 *5+1 ", " *5+3 6"),
    RULE("7 *5+2 ", " *5+3 7"),
    RULE("7 *5+3 ", " *5+3 8"),
    RULE("7 *5+4 ", " *5+3 9"),
    RULE("7 *5+5 ", " *5+4 0"),

    RULE("8 *5+0 ", " *5+4 0"),
    RULE("8 *5+1 ", " *5+4 1"),
    RULE("8 *5+2 ", " *5+4 2"),
    RULE("8 *5+3 ", " *5+4 3"),
    RULE("8 *5+4 ", " *5+4 4"),
    RULE("8 *5+5 ", " *5+4 5"),

    RULE("9 *5+0 ", " *5+4 5"),
    RULE("9 *5+1 ", " *5+4 6"),
    RULE("9 *5+2 ", " *5+4 7"),
    RULE("9 *5+3 ", " *5+4 8"),
    RULE("9 *5+4 ", " *5+4 9"),
    RULE("9 *5+5 ", " *5+5 0"),

    // finish at < (implicit leading 0)
    RULE("< *5+0 ", " +1 <"),
    RULE("< *5+1 ", " +1 <1"),
    RULE("< *5+2 ", " +1 <2"),
    RULE("< *5+3 ", " +1 <3"),
    RULE("< *5+4 ", " +1 <4"),
    RULE("< *5+5 ", " +1 <5"),

    ////////////////////////////
    // increment after iteration

    RULE("0 +1 ", " +0 1"),
    RULE("1 +1 ", " +0 2"),
    RULE("2 +1 ", " +0 3"),
    RULE("3 +1 ", " +0 4"),
    RULE("4 +1 ", " +0 5"),
    RULE("5 +1 ", " +0 6"),
    RULE("6 +1 ", " +0 7"),
    RULE("7 +1 ", " +0 8"),
    RULE("8 +1 ", " +0 9"),
    RULE("9 +1 ", " +1 0"),
    // reached implicit leading 0 at the begin of string
    RULE(" +1 ", " +0 1"),

    //////////////////////
    RULE(" +0 ", ""),
    //////////////////////
    // stop on unit

    FINAL_RULE("0<1>", "0"),
    FINAL_RULE("1<1>", "1"),
    FINAL_RULE("2<1>", "2"),
    FINAL_RULE("3<1>", "3"),
    FINAL_RULE("4<1>", "4"),
    FINAL_RULE("5<1>", "5"),
    FINAL_RULE("6<1>", "6"),
    FINAL_RULE("7<1>", "7"),
    FINAL_RULE("8<1>", "8"),
    FINAL_RULE("9<1>", "9"),

    FINAL_RULE("<1>", "0"), // immediate stop will return 0

    ///////////////////////
    // start

    RULE(">", ":>"),
);

TEST(decimal, seven) {
    auto cb = [](std::string const& x, std::string const& y) {
        std::cout << x << " -> " << y << std::endl;
    };
    EXPECT_EQ(decimal("{<27>}", cb).first, "{111}");
}