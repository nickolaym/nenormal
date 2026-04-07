#include <concepts>
#include <iostream>
#include <iomanip>
#include <algorithm>

#define REPRESENTS(Name) static constexpr bool this_is_##Name = true;
#define CONCEPT(Name) template<class T> concept Name = (T::this_is_##Name == true);

#define PRINTVALUE(type, prefix, suffix) \
    friend std::ostream& operator << (std::ostream& os, type const& v) { \
        os << prefix << v.value << suffix; \
        return os; \
    }

// ct - compile-time
template<auto V> struct ct {
    using type = decltype(V);
    static constexpr auto value = V;
    REPRESENTS(Ct)
    friend std::ostream& operator << (std::ostream& os, ct const& v) {
        os << "ct<" << v.value << ">";
        return os;
    }
};
CONCEPT(Ct)

template<size_t N> using charbuf = char[N];

template<size_t N> struct str {
    static_assert(N > 0); // nul-terminated string
    charbuf<N> value = {};
    constexpr str() = default;
    constexpr str(const charbuf<N>& v) { std::copy(std::begin(v), std::end(v), std::begin(value)); }

    REPRESENTS(Str)
    friend std::ostream& operator << (std::ostream& os, str const& v) {
        os << std::quoted(v.value) << "_ss";
        return os;
    }

    constexpr auto begin() { return std::begin(value); }
    constexpr auto begin() const { return std::begin(value); }
    constexpr auto end() { return std::begin(value) + size(); }
    constexpr auto end() const { return std::begin(value) + size(); }

    constexpr size_t size() const { return N-1; }

    constexpr bool operator == (const str& rhs) const = default;
    template<size_t M> constexpr bool operator == (const str<M>& rhs) const { return false; }
};
// CTAD
template<size_t N> str(const charbuf<N>&) -> str<N>;

CONCEPT(Str)
template<class T> concept CtStr = Ct<T> && Str<typename T::type>;

// string literals
template<str s> constexpr auto operator""_ss() { return s; }
template<str s> constexpr auto operator""_cts() { return ct<s>{}; }

// post function types

struct fail {
    friend std::ostream& operator << (std::ostream& os, fail const& v) { os << "fail"; return os; }

    constexpr bool operator == (const fail&) const = default;
    constexpr bool operator == (const auto&) const { return false; }
};
constexpr bool failed(const auto& a) { return fail{} == a; }

template<class T> concept Fail = std::same_as<T, fail>;
template<class T> concept StrOrFail = Str<T> || Fail<T>;


// single search-and-replace function
CONCEPT(Post)

template<Str auto s, Str auto r> struct post {
    REPRESENTS(Post)
    static_assert(s != str{""}, "empty search string results in an endless loop");
    static_assert(s != r, "same search and replace results in an endless loop");
    friend std::ostream& operator << (std::ostream& os, post const& v) {
        os << "post(" << v.s << " -> " << v.r << ")";
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
template<class T> struct the_fun;

template<class F> struct the_fun {
    F fun;
    constexpr auto operator()(auto a) const { return fun(a); }
};

template<class T> struct the_arg {
    static constexpr bool stopped = false;
    T value;

    template<class F>
    constexpr auto operator|(the_fun<F> f) const {
        auto r = f(value);
        if constexpr(failed(r)) return *this; // retry
        else return the_stop{r}; // stop with success
    }

    template<class F>
    constexpr auto operator^(the_fun<F> f) const {
        auto r = f(value);
        if constexpr(failed(r)) return the_stop{value}; // stop with failure
        else return the_arg<decltype(r)>{r}; // continue
    }

    constexpr auto complete() const { return fail{}; } // if not stopped yet, return fail
};

template<class T> struct the_stop {
    static constexpr bool stopped = true;
    T value;

    template<class F>
    constexpr auto operator|(the_fun<F> f) const {
        return *this; // already stopped
    }

    template<class F>
    constexpr auto operator^(the_fun<F> f) const {
        return *this; // already stopped
    }

    constexpr auto complete() const { return value; }
};

// self unit test of folds

constexpr bool test_disjunction() {
    constexpr auto a = "a"_ss;
    constexpr auto f = [](auto) { return fail{}; };
    constexpr auto g = [](auto  x) { return "g"_ss; };
    constexpr auto h = [](auto  x) { return "h"_ss; };

    constexpr auto ta = the_arg{a};
    constexpr auto tf = the_fun{f};
    constexpr auto tg = the_fun{g};
    constexpr auto th = the_fun{h};

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
    constexpr auto a = "a"_ss;
    constexpr auto f = [](auto) { return fail{}; };
    constexpr auto g = [](auto  x) { return "g"_ss; };
    constexpr auto h = [](auto  x) { return "h"_ss; };

    constexpr auto ta = the_arg{a};
    constexpr auto tf = the_fun{f};
    constexpr auto tg = the_fun{g};
    constexpr auto th = the_fun{h};

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

template<Post auto... ps> struct posts {
    REPRESENTS(Post)
    constexpr auto operator()(CtStr auto t) const {
        return ((the_arg{t} | ... | the_fun{ps})).complete();
    }
};
template<> struct posts<> {
    REPRESENTS(Post)
    constexpr auto operator()(CtStr auto t) const { return fail{}; }
};

// To hide a program from compiler output
#define NAMED_POST(name, p) \
(struct name { \
    REPRESENTS(Post) \
    constexpr auto operator()(CtStr auto t) const { return (p)(t); } \
}){}

// loop

template<Post auto p> struct postloop {
    REPRESENTS(Post)
    constexpr auto operator()(CtStr auto t) const {
        constexpr auto res = the_arg{t} ^ the_fun{p};
        if constexpr (res.stopped) {
            return res.value;
        } else {
            return postloop{}(res.value);
        }
    }
};

// arithmetics over the Post's machine
// 123+45=
//
// move digit to the left
// x+ -> +[x]
// [x]y -> y[x]

#define STR(s) (str{s}) // s##_ss
#define CTSTR(s) (ct<STR(s)>{}) // s##_cts
#define POST(s, r) (post<STR(s), STR(r)>{})
#define POSTS(...) posts<__VA_ARGS__>{}

constexpr auto take_digit = POSTS(
    POST("0+", "+[0]"),
    POST("1+", "+[1]"),
    POST("2+", "+[2]"),
    POST("3+", "+[3]"),
    POST("4+", "+[4]"),
    POST("5+", "+[5]"),
    POST("6+", "+[6]"),
    POST("7+", "+[7]"),
    POST("8+", "+[8]"),
    POST("9+", "+[9]"),
    posts<>{}
);

constexpr auto move_digit_to_the_right = POSTS(
    POST("[0]0", "0[0]"),
    POST("[0]1", "1[0]"),
    POST("[0]2", "2[0]"),
    POST("[0]3", "3[0]"),
    POST("[0]4", "4[0]"),
    POST("[0]5", "5[0]"),
    POST("[0]6", "6[0]"),
    POST("[0]7", "7[0]"),
    POST("[0]8", "8[0]"),
    POST("[0]9", "9[0]"),

    POST("[1]0", "0[1]"),
    POST("[1]1", "1[1]"),
    POST("[1]2", "2[1]"),
    POST("[1]3", "3[1]"),
    POST("[1]4", "4[1]"),
    POST("[1]5", "5[1]"),
    POST("[1]6", "6[1]"),
    POST("[1]7", "7[1]"),
    POST("[1]8", "8[1]"),
    POST("[1]9", "9[1]"),

    POST("[2]0", "0[2]"),
    POST("[2]1", "1[2]"),
    POST("[2]2", "2[2]"),
    POST("[2]3", "3[2]"),
    POST("[2]4", "4[2]"),
    POST("[2]5", "5[2]"),
    POST("[2]6", "6[2]"),
    POST("[2]7", "7[2]"),
    POST("[2]8", "8[2]"),
    POST("[2]9", "9[2]"),

    POST("[3]0", "0[3]"),
    POST("[3]1", "1[3]"),
    POST("[3]2", "2[3]"),
    POST("[3]3", "3[3]"),
    POST("[3]4", "4[3]"),
    POST("[3]5", "5[3]"),
    POST("[3]6", "6[3]"),
    POST("[3]7", "7[3]"),
    POST("[3]8", "8[3]"),
    POST("[3]9", "9[3]"),

    POST("[4]0", "0[4]"),
    POST("[4]1", "1[4]"),
    POST("[4]2", "2[4]"),
    POST("[4]3", "3[4]"),
    POST("[4]4", "4[4]"),
    POST("[4]5", "5[4]"),
    POST("[4]6", "6[4]"),
    POST("[4]7", "7[4]"),
    POST("[4]8", "8[4]"),
    POST("[4]9", "9[4]"),

    POST("[5]0", "0[5]"),
    POST("[5]1", "1[5]"),
    POST("[5]2", "2[5]"),
    POST("[5]3", "3[5]"),
    POST("[5]4", "4[5]"),
    POST("[5]5", "5[5]"),
    POST("[5]6", "6[5]"),
    POST("[5]7", "7[5]"),
    POST("[5]8", "8[5]"),
    POST("[5]9", "9[5]"),

    POST("[6]0", "0[6]"),
    POST("[6]1", "1[6]"),
    POST("[6]2", "2[6]"),
    POST("[6]3", "3[6]"),
    POST("[6]4", "4[6]"),
    POST("[6]5", "5[6]"),
    POST("[6]6", "6[6]"),
    POST("[6]7", "7[6]"),
    POST("[6]8", "8[6]"),
    POST("[6]9", "9[6]"),

    POST("[7]0", "0[7]"),
    POST("[7]1", "1[7]"),
    POST("[7]2", "2[7]"),
    POST("[7]3", "3[7]"),
    POST("[7]4", "4[7]"),
    POST("[7]5", "5[7]"),
    POST("[7]6", "6[7]"),
    POST("[7]7", "7[7]"),
    POST("[7]8", "8[7]"),
    POST("[7]9", "9[7]"),

    POST("[8]0", "0[8]"),
    POST("[8]1", "1[8]"),
    POST("[8]2", "2[8]"),
    POST("[8]3", "3[8]"),
    POST("[8]4", "4[8]"),
    POST("[8]5", "5[8]"),
    POST("[8]6", "6[8]"),
    POST("[8]7", "7[8]"),
    POST("[8]8", "8[8]"),
    POST("[8]9", "9[8]"),

    POST("[9]0", "0[9]"),
    POST("[9]1", "1[9]"),
    POST("[9]2", "2[9]"),
    POST("[9]3", "3[9]"),
    POST("[9]4", "4[9]"),
    POST("[9]5", "5[9]"),
    POST("[9]6", "6[9]"),
    POST("[9]7", "7[9]"),
    POST("[9]8", "8[9]"),
    POST("[9]9", "9[9]"),

    posts<>{}
);

constexpr auto add_digits = POSTS(
    POST("0[0]=", "=0"),
    POST("1[0]=", "=1"),
    POST("2[0]=", "=2"),
    POST("3[0]=", "=3"),
    POST("4[0]=", "=4"),
    POST("5[0]=", "=5"),
    POST("6[0]=", "=6"),
    POST("7[0]=", "=7"),
    POST("8[0]=", "=8"),
    POST("9[0]=", "=9"),

    POST("0[1]=", "=1"),
    POST("1[1]=", "=2"),
    POST("2[1]=", "=3"),
    POST("3[1]=", "=4"),
    POST("4[1]=", "=5"),
    POST("5[1]=", "=6"),
    POST("6[1]=", "=7"),
    POST("7[1]=", "=8"),
    POST("8[1]=", "=9"),
    POST("9[1]=", "^=0"),

    POST("0[2]=", "=2"),
    POST("1[2]=", "=3"),
    POST("2[2]=", "=4"),
    POST("3[2]=", "=5"),
    POST("4[2]=", "=6"),
    POST("5[2]=", "=7"),
    POST("6[2]=", "=8"),
    POST("7[2]=", "=9"),
    POST("8[2]=", "^=0"),
    POST("9[2]=", "^=1"),

    POST("0[3]=", "=3"),
    POST("1[3]=", "=4"),
    POST("2[3]=", "=5"),
    POST("3[3]=", "=6"),
    POST("4[3]=", "=7"),
    POST("5[3]=", "=8"),
    POST("6[3]=", "=9"),
    POST("7[3]=", "^=0"),
    POST("8[3]=", "^=1"),
    POST("9[3]=", "^=2"),

    POST("0[4]=", "=4"),
    POST("1[4]=", "=5"),
    POST("2[4]=", "=6"),
    POST("3[4]=", "=7"),
    POST("4[4]=", "=8"),
    POST("5[4]=", "=9"),
    POST("6[4]=", "^=0"),
    POST("7[4]=", "^=1"),
    POST("8[4]=", "^=2"),
    POST("9[4]=", "^=3"),

    POST("0[5]=", "=5"),
    POST("1[5]=", "=6"),
    POST("2[5]=", "=7"),
    POST("3[5]=", "=8"),
    POST("4[5]=", "=9"),
    POST("5[5]=", "^=0"),
    POST("6[5]=", "^=1"),
    POST("7[5]=", "^=2"),
    POST("8[5]=", "^=3"),
    POST("9[5]=", "^=4"),

    POST("0[6]=", "=6"),
    POST("1[6]=", "=7"),
    POST("2[6]=", "=8"),
    POST("3[6]=", "=9"),
    POST("4[6]=", "^=0"),
    POST("5[6]=", "^=1"),
    POST("6[6]=", "^=2"),
    POST("7[6]=", "^=3"),
    POST("8[6]=", "^=4"),
    POST("9[6]=", "^=5"),

    POST("0[7]=", "=7"),
    POST("1[7]=", "=8"),
    POST("2[7]=", "=9"),
    POST("3[7]=", "^=0"),
    POST("4[7]=", "^=1"),
    POST("5[7]=", "^=2"),
    POST("6[7]=", "^=3"),
    POST("7[7]=", "^=4"),
    POST("8[7]=", "^=5"),
    POST("9[7]=", "^=6"),

    POST("0[8]=", "=8"),
    POST("1[8]=", "=9"),
    POST("2[8]=", "^=0"),
    POST("3[8]=", "^=1"),
    POST("4[8]=", "^=2"),
    POST("5[8]=", "^=3"),
    POST("6[8]=", "^=4"),
    POST("7[8]=", "^=5"),
    POST("8[8]=", "^=6"),
    POST("9[8]=", "^=7"),

    POST("0[9]=", "=9"),
    POST("1[9]=", "^=0"),
    POST("2[9]=", "^=1"),
    POST("3[9]=", "^=2"),
    POST("4[9]=", "^=3"),
    POST("5[9]=", "^=4"),
    POST("6[9]=", "^=5"),
    POST("7[9]=", "^=6"),
    POST("8[9]=", "^=7"),
    POST("9[9]=", "^=8"),

    posts<>{}
);

constexpr auto add_carry = POSTS(
    POST("0^", "1"),
    POST("1^", "2"),
    POST("2^", "3"),
    POST("3^", "4"),
    POST("4^", "5"),
    POST("5^", "6"),
    POST("6^", "7"),
    POST("7^", "8"),
    POST("8^", "9"),
    POST("9^", "^0"),

    POST("+^", "+1"),
    posts<>{}
);

constexpr auto cleanup = POSTS(
    POST("+", ""),
    POST("=", ""),
    posts<>{}
);

constexpr auto program = NAMED_POST(program_t, POSTS(
    add_carry,
    add_digits,
    move_digit_to_the_right,
    take_digit,
    cleanup,
    posts{}
));

int main() {
    constexpr auto t = CTSTR("98765+66666=");
    constexpr auto r = postloop<program>{}(t);
    std::cout << t.value << std::endl;
    std::cout << " --> " << r.value << std::endl;
    return 0;
}
