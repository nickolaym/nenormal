// #include <concepts>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "nenormal/nenormal.h"


// rule function types

struct fail {
    friend std::ostream& operator << (std::ostream& os, fail const& v) { os << "fail"; return os; }

    constexpr bool operator == (const fail&) const = default;
    constexpr bool operator == (const auto&) const { return false; }
};
constexpr bool failed(const auto& a) { return fail{} == a; }

template<class T> concept Fail = std::same_as<T, fail>;
template<class T> concept StrOrFail = Str<T> || Fail<T>;


// single search-and-replace function
CONCEPT(Rule)

template<Str auto s, Str auto r> struct rule {
    REPRESENTS(Rule)
    static_assert(s != str{""}, "empty search string results in an endless loop");
    static_assert(s != r, "same search and replace results in an endless loop");
    friend std::ostream& operator << (std::ostream& os, rule const& v) {
        os << "rule(" << v.s << " -> " << v.r << ")";
        return os;
    }

    constexpr auto operator()(CtStr auto t) const {
        constexpr auto& src = t.value;
        if constexpr (src.size() < s.size()) {
            return fail{};
        } else if constexpr (src == s) {
            return r;
        } else {
            constexpr auto fbegin = std::search(src.begin(), src.end(), s.begin(), s.end());
            if constexpr (fbegin == src.end()) {
                return fail{};
            } else {
                return ct<
                    [&]{
                        auto fend = fbegin + s.size();
                        constexpr auto len = src.size() - s.size() + r.size();
                        str<len + 1> dst;
                        auto it = dst.begin();
                        it = std::copy(src.begin(), fbegin, it);
                        it = std::copy(r.begin(), r.end(), it);
                        it = std::copy(fend, src.end(), it);
                        *it = 0;
                        return dst;
                    }()
                >{};
            }
        }
    }
};

// utility classes for fold expressions

template<class T> struct the_arg;
template<class T> struct the_stop;

template<class T> struct the_arg {
    static constexpr bool stopped = false;
    T value;

    constexpr auto operator|(auto f) const {
        auto r = f(value);
        if constexpr(failed(r)) return *this; // retry
        else return the_stop{r}; // stop with success
    }

    constexpr auto operator^(auto f) const {
        auto r = f(value);
        if constexpr(failed(r)) return the_stop{value}; // stop with failure
        else return the_arg<decltype(r)>{r}; // continue
    }

    constexpr auto complete() const { return fail{}; } // if not stopped yet, return fail
};

template<class T> struct the_stop {
    static constexpr bool stopped = true;
    T value;

    constexpr auto operator|(auto f) const {
        return *this; // already stopped
    }

    constexpr auto operator^(auto f) const {
        return *this; // already stopped
    }

    constexpr auto complete() const { return value; }
};

// self unit test of folds

constexpr bool test_disjunction() {
    constexpr auto ta = the_arg{"a"_ss};
    constexpr auto tf = [](auto) { return fail{}; };
    constexpr auto tg = [](auto  x) { return "g"_ss; };
    constexpr auto th = [](auto  x) { return "h"_ss; };

    constexpr auto tfa = ta | tf;
    static_assert(!tfa.stopped);
    static_assert(tfa.value == "a"_ss);

    constexpr auto tga = ta | tg;
    static_assert(tga.stopped);
    static_assert(tga.value == "g"_ss);

    constexpr auto tfgha = ta | tf | tg | th;
    static_assert(tfgha.stopped);
    static_assert(tfgha.value == "g"_ss);

    return true;
}
static_assert(test_disjunction());

constexpr bool test_exposing() {
    constexpr auto ta = the_arg{"a"_ss};
    constexpr auto tf = [](auto) { return fail{}; };
    constexpr auto tg = [](auto  x) { return "g"_ss; };
    constexpr auto th = [](auto  x) { return "h"_ss; };

    constexpr auto tfa = ta ^ tf;
    static_assert(tfa.stopped);
    static_assert(tfa.value == "a"_ss);

    constexpr auto tga = ta ^ tg;
    static_assert(!tga.stopped);
    static_assert(tga.value == "g"_ss);

    constexpr auto tgha = ta ^ tg ^ th;
    static_assert(!tgha.stopped);
    static_assert(tgha.value == "h"_ss);

    constexpr auto tgfa = ta ^ tg ^ tf;
    static_assert(tgfa.stopped);
    static_assert(tgfa.value == "g"_ss);

    return true;
}
static_assert(test_exposing());

// subroutine

namespace rules_helper {
    // implements disjunction on text >> rule1 >> rule2 >> ...
    
    constexpr auto make_arg(CtStr auto text) { return arg{text}; }
    constexpr auto make_fun(Rule auto rule) {
        // rule : CtStr -> CtStr | fail
        // fun  : arg<CtStr> -> arg<CtStr> | stop<CtStr>
        return [rule]<CtStr T>(arg<T> a) {
            auto r = rule(a.value);
            if constexpr (failed(r)) // not matched yet
                return a;            // continue...
            else
                return stop{r};      // success
        };
    }
    template<CtStr T> constexpr fail make_res(arg<T> a) {
        // still not resolved
        return fail{};
    }
    template<CtStr T> constexpr T make_res(stop<T> a) {
        return a.value;
    }

} // namespace rules_helper

template<Rule auto... rs> struct rules {
    REPRESENTS(Rule)

    static constexpr auto the_chain = chain(rules_helper::make_fun(rs)...);
    constexpr auto operator()(CtStr auto t) const {
        return rules_helper::make_res(the_chain(rules_helper::make_arg(t)));
    }
};
template<> struct rules<> {
    REPRESENTS(Rule)
    constexpr auto operator()(CtStr auto t) const { return fail{}; }
};

// To hide a program from compiler output
#define NAMED_RULE(name, p) \
(struct name { \
    REPRESENTS(Rule) \
    constexpr auto operator()(CtStr auto t) const { return (p)(t); } \
}){}

// loop

template<Rule auto p> struct rule_loop {
    REPRESENTS(Rule)
    constexpr auto operator()(CtStr auto t) const {
        constexpr auto res = the_arg{t} ^ p ^ p ^ p ^ p ^ p ^ p ^ p ^ p ^ p ^ p;
        if constexpr (res.stopped) {
            return res.value;
        } else {
            return rule_loop{}(res.value);
        }
    }
};

// arithmetics over the Markov machine
// 123+45=
//
// move digit to the left
// x+ -> +[x]
// [x]y -> y[x]

#define STR(s) (str{s}) // s##_ss
#define CTSTR(s) (ct<STR(s)>{}) // s##_cts
#define RULE(s, r) (rule<STR(s), STR(r)>{})
#define RULES(...) rules<__VA_ARGS__>{}

constexpr auto take_digit = RULES(
    RULE("0+", "+[0]"),
    RULE("1+", "+[1]"),
    RULE("2+", "+[2]"),
    RULE("3+", "+[3]"),
    RULE("4+", "+[4]"),
    RULE("5+", "+[5]"),
    RULE("6+", "+[6]"),
    RULE("7+", "+[7]"),
    RULE("8+", "+[8]"),
    RULE("9+", "+[9]"),
    rules<>{}
);

constexpr auto move_digit_to_the_right = RULES(
    RULE("[0]0", "0[0]"),
    RULE("[0]1", "1[0]"),
    RULE("[0]2", "2[0]"),
    RULE("[0]3", "3[0]"),
    RULE("[0]4", "4[0]"),
    RULE("[0]5", "5[0]"),
    RULE("[0]6", "6[0]"),
    RULE("[0]7", "7[0]"),
    RULE("[0]8", "8[0]"),
    RULE("[0]9", "9[0]"),

    RULE("[1]0", "0[1]"),
    RULE("[1]1", "1[1]"),
    RULE("[1]2", "2[1]"),
    RULE("[1]3", "3[1]"),
    RULE("[1]4", "4[1]"),
    RULE("[1]5", "5[1]"),
    RULE("[1]6", "6[1]"),
    RULE("[1]7", "7[1]"),
    RULE("[1]8", "8[1]"),
    RULE("[1]9", "9[1]"),

    RULE("[2]0", "0[2]"),
    RULE("[2]1", "1[2]"),
    RULE("[2]2", "2[2]"),
    RULE("[2]3", "3[2]"),
    RULE("[2]4", "4[2]"),
    RULE("[2]5", "5[2]"),
    RULE("[2]6", "6[2]"),
    RULE("[2]7", "7[2]"),
    RULE("[2]8", "8[2]"),
    RULE("[2]9", "9[2]"),

    RULE("[3]0", "0[3]"),
    RULE("[3]1", "1[3]"),
    RULE("[3]2", "2[3]"),
    RULE("[3]3", "3[3]"),
    RULE("[3]4", "4[3]"),
    RULE("[3]5", "5[3]"),
    RULE("[3]6", "6[3]"),
    RULE("[3]7", "7[3]"),
    RULE("[3]8", "8[3]"),
    RULE("[3]9", "9[3]"),

    RULE("[4]0", "0[4]"),
    RULE("[4]1", "1[4]"),
    RULE("[4]2", "2[4]"),
    RULE("[4]3", "3[4]"),
    RULE("[4]4", "4[4]"),
    RULE("[4]5", "5[4]"),
    RULE("[4]6", "6[4]"),
    RULE("[4]7", "7[4]"),
    RULE("[4]8", "8[4]"),
    RULE("[4]9", "9[4]"),

    RULE("[5]0", "0[5]"),
    RULE("[5]1", "1[5]"),
    RULE("[5]2", "2[5]"),
    RULE("[5]3", "3[5]"),
    RULE("[5]4", "4[5]"),
    RULE("[5]5", "5[5]"),
    RULE("[5]6", "6[5]"),
    RULE("[5]7", "7[5]"),
    RULE("[5]8", "8[5]"),
    RULE("[5]9", "9[5]"),

    RULE("[6]0", "0[6]"),
    RULE("[6]1", "1[6]"),
    RULE("[6]2", "2[6]"),
    RULE("[6]3", "3[6]"),
    RULE("[6]4", "4[6]"),
    RULE("[6]5", "5[6]"),
    RULE("[6]6", "6[6]"),
    RULE("[6]7", "7[6]"),
    RULE("[6]8", "8[6]"),
    RULE("[6]9", "9[6]"),

    RULE("[7]0", "0[7]"),
    RULE("[7]1", "1[7]"),
    RULE("[7]2", "2[7]"),
    RULE("[7]3", "3[7]"),
    RULE("[7]4", "4[7]"),
    RULE("[7]5", "5[7]"),
    RULE("[7]6", "6[7]"),
    RULE("[7]7", "7[7]"),
    RULE("[7]8", "8[7]"),
    RULE("[7]9", "9[7]"),

    RULE("[8]0", "0[8]"),
    RULE("[8]1", "1[8]"),
    RULE("[8]2", "2[8]"),
    RULE("[8]3", "3[8]"),
    RULE("[8]4", "4[8]"),
    RULE("[8]5", "5[8]"),
    RULE("[8]6", "6[8]"),
    RULE("[8]7", "7[8]"),
    RULE("[8]8", "8[8]"),
    RULE("[8]9", "9[8]"),

    RULE("[9]0", "0[9]"),
    RULE("[9]1", "1[9]"),
    RULE("[9]2", "2[9]"),
    RULE("[9]3", "3[9]"),
    RULE("[9]4", "4[9]"),
    RULE("[9]5", "5[9]"),
    RULE("[9]6", "6[9]"),
    RULE("[9]7", "7[9]"),
    RULE("[9]8", "8[9]"),
    RULE("[9]9", "9[9]"),

    rules<>{}
);

constexpr auto add_digits = RULES(
    RULE("0[0]=", "=0"),
    RULE("1[0]=", "=1"),
    RULE("2[0]=", "=2"),
    RULE("3[0]=", "=3"),
    RULE("4[0]=", "=4"),
    RULE("5[0]=", "=5"),
    RULE("6[0]=", "=6"),
    RULE("7[0]=", "=7"),
    RULE("8[0]=", "=8"),
    RULE("9[0]=", "=9"),

    RULE("0[1]=", "=1"),
    RULE("1[1]=", "=2"),
    RULE("2[1]=", "=3"),
    RULE("3[1]=", "=4"),
    RULE("4[1]=", "=5"),
    RULE("5[1]=", "=6"),
    RULE("6[1]=", "=7"),
    RULE("7[1]=", "=8"),
    RULE("8[1]=", "=9"),
    RULE("9[1]=", "^=0"),

    RULE("0[2]=", "=2"),
    RULE("1[2]=", "=3"),
    RULE("2[2]=", "=4"),
    RULE("3[2]=", "=5"),
    RULE("4[2]=", "=6"),
    RULE("5[2]=", "=7"),
    RULE("6[2]=", "=8"),
    RULE("7[2]=", "=9"),
    RULE("8[2]=", "^=0"),
    RULE("9[2]=", "^=1"),

    RULE("0[3]=", "=3"),
    RULE("1[3]=", "=4"),
    RULE("2[3]=", "=5"),
    RULE("3[3]=", "=6"),
    RULE("4[3]=", "=7"),
    RULE("5[3]=", "=8"),
    RULE("6[3]=", "=9"),
    RULE("7[3]=", "^=0"),
    RULE("8[3]=", "^=1"),
    RULE("9[3]=", "^=2"),

    RULE("0[4]=", "=4"),
    RULE("1[4]=", "=5"),
    RULE("2[4]=", "=6"),
    RULE("3[4]=", "=7"),
    RULE("4[4]=", "=8"),
    RULE("5[4]=", "=9"),
    RULE("6[4]=", "^=0"),
    RULE("7[4]=", "^=1"),
    RULE("8[4]=", "^=2"),
    RULE("9[4]=", "^=3"),

    RULE("0[5]=", "=5"),
    RULE("1[5]=", "=6"),
    RULE("2[5]=", "=7"),
    RULE("3[5]=", "=8"),
    RULE("4[5]=", "=9"),
    RULE("5[5]=", "^=0"),
    RULE("6[5]=", "^=1"),
    RULE("7[5]=", "^=2"),
    RULE("8[5]=", "^=3"),
    RULE("9[5]=", "^=4"),

    RULE("0[6]=", "=6"),
    RULE("1[6]=", "=7"),
    RULE("2[6]=", "=8"),
    RULE("3[6]=", "=9"),
    RULE("4[6]=", "^=0"),
    RULE("5[6]=", "^=1"),
    RULE("6[6]=", "^=2"),
    RULE("7[6]=", "^=3"),
    RULE("8[6]=", "^=4"),
    RULE("9[6]=", "^=5"),

    RULE("0[7]=", "=7"),
    RULE("1[7]=", "=8"),
    RULE("2[7]=", "=9"),
    RULE("3[7]=", "^=0"),
    RULE("4[7]=", "^=1"),
    RULE("5[7]=", "^=2"),
    RULE("6[7]=", "^=3"),
    RULE("7[7]=", "^=4"),
    RULE("8[7]=", "^=5"),
    RULE("9[7]=", "^=6"),

    RULE("0[8]=", "=8"),
    RULE("1[8]=", "=9"),
    RULE("2[8]=", "^=0"),
    RULE("3[8]=", "^=1"),
    RULE("4[8]=", "^=2"),
    RULE("5[8]=", "^=3"),
    RULE("6[8]=", "^=4"),
    RULE("7[8]=", "^=5"),
    RULE("8[8]=", "^=6"),
    RULE("9[8]=", "^=7"),

    RULE("0[9]=", "=9"),
    RULE("1[9]=", "^=0"),
    RULE("2[9]=", "^=1"),
    RULE("3[9]=", "^=2"),
    RULE("4[9]=", "^=3"),
    RULE("5[9]=", "^=4"),
    RULE("6[9]=", "^=5"),
    RULE("7[9]=", "^=6"),
    RULE("8[9]=", "^=7"),
    RULE("9[9]=", "^=8"),

    rules<>{}
);

constexpr auto add_carry = RULES(
    RULE("0^", "1"),
    RULE("1^", "2"),
    RULE("2^", "3"),
    RULE("3^", "4"),
    RULE("4^", "5"),
    RULE("5^", "6"),
    RULE("6^", "7"),
    RULE("7^", "8"),
    RULE("8^", "9"),
    RULE("9^", "^0"),

    RULE("+^", "+1"),
    rules<>{}
);

constexpr auto cleanup = RULES(
    RULE("+", ""),
    RULE("=", ""),
    rules<>{}
);

constexpr auto program = NAMED_RULE(program_t, RULES(
    add_carry,
    add_digits,
    move_digit_to_the_right,
    take_digit,
    cleanup,
    rules{}
));

int main() {
    constexpr auto t = CTSTR("98765+66666=");
    constexpr auto r = rule_loop<program>{}(t);
    std::cout << t.value << std::endl;
    std::cout << " --> " << r.value << std::endl;
    return 0;
}
