#include "nenormal/nenormal.h"
#include <gtest/gtest.h>

// monad Maybe

CONCEPT(Nothing); // stops a chain
CONCEPT(Just); // continues a chain
template<class T> concept Maybe = Nothing<T> || Just<T>;

struct nothing {
    REPRESENTS(Nothing);
    constexpr operator bool() const { return false; }
    constexpr auto then(auto f, auto e) const { return e; }
};

template<class T>
struct just {
    REPRESENTS(Just);
    using type = T;
    T value;
    constexpr operator bool() const { return true; }
    constexpr auto then(auto f, auto e) const { return f(value); }
};


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

    constexpr auto either(auto lf, auto rf) const {
        return lf(value);
    }
    constexpr auto eitherLifted(auto lf, auto rf) const {
        auto res = lf(value);
        return left_type<decltype(res)>{res};
    }
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

    constexpr auto either(auto lf, auto rf) const {
        return rf(value);
    }
    constexpr auto eitherLifted(auto lf, auto rf) const {
        auto res = rf(value);
        return right_type<decltype(res)>{res};
    }
};

// named constructors of types
// because of ambiguity of C++ syntax
// left_type{x} means
// either left_type<decltype(x)>{.value = x}
// or copy of x, if x is left_type<T> itself.

// also we need them as object entities (see below)
constexpr auto left = []<class T>(T t) { return left_type<T>{t}; };
constexpr auto right = []<class T>(T t) { return right_type<T>{t}; };

// deconstructor of Either (left only, because right values are temporary)
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

// somehow implement following flow
// input: (data, aux)
// replacer : data -> maybe data'
// kind: regular/final = right/left

template<class T> concept JustCtStr = Just<T> && CtStr<typename T::type>;
template<class T> concept MaybeCtStr = Nothing<T> || JustCtStr<T>;

// maybe_subst_fun uses common Maybe instead of ad-hoc fail|value
template<Str auto S, Str auto R, rule_state_t C>
constexpr auto maybe_subst_fun(rule<S,R,C> r) {
    return [](CtStr auto data) -> MaybeCtStr auto {
        FailOrSubst auto result = rule<S,R,C>::try_substitute(data);
        if constexpr (failed(result))
            return nothing{};
        else
            return just{result};
    };
}

// mapping ad-hoc rule_state_t constants to constructors of Either as we need
template<class> struct incomplete;
template<rule_state_t s> constexpr auto rule_output_kind = incomplete<ct<s>>{};
template<> constexpr auto rule_output_kind<regular_state> = right;
template<> constexpr auto rule_output_kind<final_state> = left;

// implement rule function using monadic tools only
// (still with extract_text / update_text, TODO rewrite that)
template<Str auto S, Str auto R, rule_state_t C>
constexpr auto monadic_rule(rule<S,R,C> r) {
    constexpr auto f = [](MonadicDomain auto data) -> MonadicOutput auto {
        constexpr auto msf = maybe_subst_fun(rule<S,R,C>{});
        constexpr auto kind = rule_output_kind<C>;
        return msf(extract_text(data)).then(
            [&](CtStr auto res) {
                return left(kind(update_text(data, rule<S,R,C>{}, res)));
            },
            right(data) // keep unchanged
        );
    };
    return f;
}

#if 0
//
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
#endif

// terminal rule for the loop
constexpr auto monadic_terminal_rule = [](MonadicDomain auto data) {
    return left(left(data));
};

// chain of alternatives
template<auto... ps>
constexpr auto monadic_rule(rules<ps...>) {
    return [](MonadicDomain auto data) -> MonadicOutput auto {
        return (right(data) >> ... >> monadic_rule(ps));
    };
}

// hidden rule passes old augmentation to new string result

constexpr auto rebind_monadic_output(MonadicDomain auto data, MonadicOutput auto res) {
    auto do_rebind_text = [&](CtStr auto v) { return rebind_text(data, v); };
#if 0
    if constexpr (Left<decltype(res)>)
        if constexpr (Left<decltype(res.value)>)
            return left(left(do_rebind_text(res.value.value)));
        else
            return left(right(do_rebind_text(res.value.value)));
    else
        return right(do_rebind_text(res.value));
#else
    return res.eitherLifted(
        // left(left(s)), left(right(s))
        [&](Either auto v) { return v.eitherLifted(do_rebind_text, do_rebind_text); },
        // right(s)
        do_rebind_text
    );
#endif
}

template<Rule auto p>
constexpr auto monadic_rule(hidden_rule<p>) {
    return [](MonadicDomain auto data) -> MonadicOutput auto {
        return rebind_monadic_output(data, right(extract_text(data)) >> monadic_rule(p));
    };
}

// rule loop
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

TEST(monadic, augmented) {
    constexpr auto augmented = [](CtStr auto src) {
        return augmented_text{src, cumulative_effect{ [](int n, auto...) { return n+1; }, 0 }};
    };

    constexpr auto rl = monadic_rule_loop<program>{};

    constexpr auto res = rl(augmented("()()()"_cts));
    static_assert(res.text == "good"_cts);
    static_assert(res.aux.a == 4);
}

TEST(monadic, with_hidden) {
    constexpr auto rl_hidden = monadic_rule_loop< HIDDEN_RULE(FINAL_RULE("","")) >{};
    constexpr auto rl_public = monadic_rule_loop<             FINAL_RULE("","")  >{};
    static_assert(rl_hidden(""_cts) == ""_cts);
    static_assert(rl_public(""_cts) == ""_cts);

    constexpr auto augmented = [](CtStr auto src) {
        return augmented_text{src, cumulative_effect{ [](int n, auto...) { return n+1; }, 0 }};
    };

    constexpr auto res_hidden = rl_hidden(augmented(""_cts));
    static_assert(res_hidden.text == ""_cts);
    static_assert(res_hidden.aux.a == 0);

    constexpr auto res_public = rl_public(augmented(""_cts));
    static_assert(res_public.text == ""_cts);
    static_assert(res_public.aux.a == 1);
}

TEST(monadic, runtime) {
    constexpr auto rl = monadic_rule_loop<program>{};
    constexpr auto augmented = [](CtStr auto src) {
        return augmented_text{
            src,
            side_effect{
                [](CtStr auto i, auto p, CtStr auto o) {
                    std::cout << std::quoted(i.value.view()) << "  >>  " << p << "  ==  " << std::quoted(o.value.view()) << std::endl;
                }
            }
        };
    };

    rl(augmented("()()()((()))"_cts));
    rl(augmented("()()())))((()))(("_cts));
}
