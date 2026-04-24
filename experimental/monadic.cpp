#include "nenormal/nenormal.h"
#include <gtest/gtest.h>

// monad Either

CONCEPT(Left); // stops a chain
CONCEPT(Right); // continues a chain
template<class T> concept Either = Left<T> || Right<T>;
// TODO: support runtime class with behavior of this monad.

template<class T> struct left_type {
    REPRESENTS(Left);
    using type = T;
    T value;
    constexpr Either auto operator >> (auto&& f) const { return *this; }

    friend std::ostream& operator << (std::ostream& ost, const left_type& t) {
        return ost << "left(" << t.value << ")";
    }

    constexpr bool operator == (const left_type& other) const = default;
};
template<class T> struct right_type {
    REPRESENTS(Right);
    using type = T;
    T value;
    constexpr Either auto operator >> (auto&& f) const { return f(value); }

    friend std::ostream& operator << (std::ostream& ost, const right_type& t) {
        return ost << "right(" << t.value << ")";
    }

    constexpr bool operator == (const right_type& other) const = default;
};

template<class T> constexpr auto left(T t) { return left_type<T>{t}; }
template<class T> constexpr auto right(T t) { return right_type<T>{t}; }

constexpr auto fromLeft(Left auto lt) { return lt.value; }

// rule input: domain
// rule output:
// - right_type< domain > means "not matched yet"
// - left_type< right_type< domain > > means "matched, continue" = left_type< not_matched_yet >
// - left_type< left_type< domain > > means "matched, stop" = left_type< finished >

template<class T> concept MonadicDomain = RuleInput<T>;

template<class T> concept MonadicFinished =
    Left<T> && MonadicDomain<typename T::type>;
template<class T> concept MonadicContinued =
    Right<T> && MonadicDomain<typename T::type>;
template<class T> concept MonadicEitherFinishedOrContinued =
    Either<T> && MonadicDomain<typename T::type>;

template<class T> concept MonadicNotMatchedYet = MonadicContinued<T>;
template<class T> concept MonadicMatchedRegular = Left<T> && MonadicContinued<typename T::type>;
template<class T> concept MonadicMatchedFinal = Left<T> && MonadicFinished<typename T::type>;

template<class T> concept MonadicMatched = MonadicMatchedRegular<T> || MonadicMatchedFinal<T>;
template<class T> concept MonadicOutput = MonadicNotMatchedYet<T> || MonadicMatched<T>;

template<auto S, auto R, auto C>
constexpr auto monadic_rule(rule<S,R,C>) {
    return [](MonadicDomain auto data) -> MonadicOutput auto {
        RuleOutput auto result = rule<S,R,C>{}(data);
        if constexpr(failed(result)) {
            return right(data); // continue matching...
        } else if constexpr(result.state.value == regular_state) {
            return left(right(result.data)); // stop matching, continue loop
        } else {
            // return left(left(result.data)); // stop matching, stop loop
            MonadicFinished auto f = left(result.data);
            MonadicMatchedFinal auto o = left(f);
            return o;
        }
    };
}

constexpr auto monadic_terminal_rule = [](MonadicDomain auto data) {
    return left(left(data));
};

template<auto... ps>
constexpr auto monadic_rule(rules<ps...>) {
    return [](MonadicDomain auto data) -> MonadicOutput auto {
        return (right(data) >> ... >> monadic_rule(ps));
    };
}

template<auto p>
struct monadic_rule_loop {
    static constexpr auto mp = monadic_rule(p);
    static constexpr auto body = [](MonadicDomain auto data) -> MonadicEitherFinishedOrContinued auto {
        // we are sure that the MonadicOutput result is always matched
        // (either with rule p or with terminal rule)
        return fromLeft(right(data) >> mp >> monadic_terminal_rule);
    };
    static constexpr auto repeated_body = [](MonadicDomain auto data) -> MonadicEitherFinishedOrContinued auto {
        return right(data)
            >> body >> body >> body >> body >> body
            >> body >> body >> body >> body >> body;
    };
    struct looped_body_t {
        constexpr MonadicFinished auto operator()(MonadicDomain auto data) const {
            return right(data) >> repeated_body >> looped_body_t{};
        }
    };
    static constexpr auto looped_body = looped_body_t{};

    constexpr MonadicDomain auto operator()(MonadicDomain auto data) const {
        return fromLeft(right(data) >> looped_body);
    }
};

//////////

// example: bracket sequence
constexpr auto program = RULES(
    RULE("()", ""),
    RULE("(", "-"),
    RULE(")", "-"),
    RULE("--", "-"),
    FINAL_RULE("-", "bad"),
    FINAL_RULE("", "good")
);

TEST(monadic, elementary) {
    constexpr auto mp = monadic_rule(RULE("a", "b"));
    constexpr auto mf = monadic_rule(FINAL_RULE("a", "b"));

    constexpr MonadicNotMatchedYet auto input_empty = right(""_cts);
    constexpr MonadicNotMatchedYet auto input_aaa = right("aaa"_cts);

    constexpr MonadicMatchedRegular auto regular_baa = left(right("baa"_cts));
    constexpr MonadicMatchedFinal auto final_baa = left(left("baa"_cts));

    static_assert((input_empty >> mp) == input_empty);
    static_assert((input_aaa >> mp) == regular_baa);
    static_assert((input_aaa >> mf) == final_baa);
}

TEST(monadic, single_step) {
    constexpr auto mp = monadic_rule(program);
    static_assert((right(""_cts) >> mp) == left(left("good"_cts)));
    static_assert((right("-"_cts) >> mp) == left(left("bad"_cts)));
    static_assert((right(")()("_cts) >> mp) == left(right(")("_cts)));
    static_assert((right("))"_cts) >> mp) == left(right("-)"_cts)));
    static_assert((right("(("_cts) >> mp) == left(right("-("_cts)));
    static_assert((right("---"_cts) >> mp) == left(right("--"_cts)));
}

TEST(monadic, simple_loop) {
    constexpr auto p = RULE("()", "");
    constexpr auto rl = monadic_rule_loop<p>{};

    constexpr auto input3 = right("((()))"_cts);
    constexpr auto input12 = right("(((((((((((())))))))))))"_cts);
    constexpr auto input22 = right("()()()()()()()()()()()()()()()()()()()()()()"_cts);

    static_assert((input3 >> rl.mp) == left(right("(())"_cts)));
    static_assert((input3 >> rl.body) == right("(())"_cts));

    static_assert((input3 >> rl.repeated_body) == left(""_cts));
    static_assert((input12 >> rl.repeated_body) == right("(())"_cts));
    static_assert((input12 >> rl.looped_body) == left(""_cts));
    static_assert((input22 >> rl.looped_body) == left(""_cts));

    static_assert(rl(input3.value) == ""_cts);
    static_assert(rl(input22.value) == ""_cts);
    static_assert(rl(")))()()()"_cts) == ")))"_cts);
}

TEST(monadic, program) {
    constexpr auto rl = monadic_rule_loop<program>{};
    static_assert(rl("()()()"_cts) == "good"_cts);
    static_assert(rl("()()))((()"_cts) == "bad"_cts);
}
