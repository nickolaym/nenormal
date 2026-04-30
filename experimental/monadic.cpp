#include "nenormal/nenormal.h"
#include <gtest/gtest.h>

// ad-hoc stateful:
// extract_text(Stateful) -> Str
// update_text(Stateful, Rule, Str) -> Stateful
// rebind_text(Stateful, Str) -> Str

// extract_handler(Stateful) -> Handler
// Handler: (Str, Rule, Str) -> Handler
// construct_state(Handler, Str) -> Stateful

template<class T, class A>
struct stateful {
    REPRESENTS(Stateful)
    using type = T;
    using aux_type = A;

    T value;
    A aux;
};
CONCEPT(Stateful);

// rule input: domain
// rule output:
// - right_type< domain > means "not matched yet"
// - left_type< right_type< domain > > means "matched, continue" = left_type< not_matched_yet >
// - left_type< left_type< domain > > means "matched, stop" = left_type< finished >

template<class T> concept MonadicDomain = Stateful<T> && CtStr<typename T::type> && Augmentation<typename T::aux_type>;

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

template<class T> concept OutputLike = (Left<T> && Either<typename T::type>) || Right<T>;

constexpr OutputLike auto output_map(auto f, OutputLike auto o) {
    return o.eitherLifted(
        [&](Either auto l) { return l.eitherLifted(f, f); },
        f);
}

constexpr MonadicNotMatchedYet auto simple_in(CtStr auto i) {
    return right(stateful{i, empty{}});
}
constexpr auto simplify_res = [](Stateful auto o) { return o.value; };
constexpr OutputLike auto simplify_ruleout(MonadicOutput auto o) {
    static_assert(OutputLike<decltype(o)>);
    return output_map(simplify_res, o);
}
constexpr Either auto simplify_loopout(MonadicEitherFinishedOrContinued auto o) {
    return o.eitherLifted(simplify_res, simplify_res);
}

// somehow implement following flow
// input: stateful(str, aux)
// replacer : str -> maybe str'
// kind: regular/final = right/left

// mapping ad-hoc rule_state_t constants to constructors of Either as we need
template<class> struct incomplete;
template<rule_state_t s> constexpr auto rule_output_kind = incomplete<ct<s>>{};
template<> constexpr auto rule_output_kind<regular_state> = right;
template<> constexpr auto rule_output_kind<final_state> = left;

// implement rule function using monadic tools only
// (still with extract_text / update_text, TODO rewrite that)
template<Str auto S, Str auto R, rule_state_t C>
constexpr auto monadic_rule(rule<S,R,C> r) {
    constexpr auto f = [](MonadicDomain auto src) -> MonadicOutput auto {
        constexpr auto kind = rule_output_kind<C>;

        CtStr auto src_text = src.value;
        return try_substitute(ct<S>{}, ct<R>{}, src_text).then(
            [&](CtStr auto dst_text) {
                auto src_handler = src.aux;
                auto dst_handler = src_handler(src_text, rule<S,R,C>{}, dst_text);
                auto dst = stateful{dst_text, dst_handler};
                return left(kind(dst));
            },
            right(src) // keep unchanged
        );
    };
    return f;
}

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
    auto old_aux = data.aux;
    return output_map(
        [&](MonadicDomain auto d) { return stateful{d.value, data.aux}; },
        res);
}

template<Rule auto p>
constexpr auto monadic_rule(hidden_rule<p>) {
    return [](MonadicDomain auto data) -> MonadicOutput auto {
        CtStr auto src = data.value;
        MonadicDomain auto simple_data = stateful{src, empty{}};
        return rebind_monadic_output(data, right(simple_data) >> monadic_rule(p));
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

    constexpr CtStr auto simple(CtStr auto s) const {
        return simplify_res(fromLeft(simple_in(s) >> looped_body));
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

    constexpr MonadicNotMatchedYet auto input_empty = simple_in(""_cts);
    constexpr MonadicNotMatchedYet auto input_aaa = simple_in("aaa"_cts);

    static_assert(simplify_ruleout(input_empty >> mp) == right(""_cts));
    static_assert(simplify_ruleout(input_aaa >> mp) == left(right("baa"_cts)));
    static_assert(simplify_ruleout(input_aaa >> mf) == left(left("baa"_cts)));
}

TEST(monadic, single_step) {
    constexpr auto mp = monadic_rule(program);
    static_assert(simplify_ruleout(simple_in(""_cts) >> mp) == left(left("good"_cts)));
    static_assert(simplify_ruleout(simple_in("-"_cts) >> mp) == left(left("bad"_cts)));
    static_assert(simplify_ruleout(simple_in(")()("_cts) >> mp) == left(right(")("_cts)));
    static_assert(simplify_ruleout(simple_in("))"_cts) >> mp) == left(right("-)"_cts)));
    static_assert(simplify_ruleout(simple_in("(("_cts) >> mp) == left(right("-("_cts)));
    static_assert(simplify_ruleout(simple_in("---"_cts) >> mp) == left(right("--"_cts)));
}

TEST(monadic, simple_loop) {
    constexpr auto p = RULE("()", "");
    constexpr auto rl = monadic_rule_loop<p>{};

    constexpr auto input3 = simple_in("((()))"_cts);
    constexpr auto input12 = simple_in("(((((((((((())))))))))))"_cts);
    constexpr auto input22 = simple_in("()()()()()()()()()()()()()()()()()()()()()()"_cts);

    static_assert(simplify_ruleout(input3 >> rl.mp) == left(right("(())"_cts)));
    static_assert(simplify_ruleout(input3 >> rl.body) == right("(())"_cts));

    static_assert(simplify_loopout(input3 >> rl.repeated_body) == left(""_cts));
    static_assert(simplify_loopout(input12 >> rl.repeated_body) == right("(())"_cts));
    static_assert(simplify_loopout(input12 >> rl.looped_body) == left(""_cts));
    static_assert(simplify_loopout(input22 >> rl.looped_body) == left(""_cts));

    static_assert(rl.simple(input3.value.value) == ""_cts);
    static_assert(rl.simple(input22.value.value) == ""_cts);
    static_assert(rl.simple(")))()()()"_cts) == ")))"_cts);
}

TEST(monadic, program) {
    constexpr auto rl = monadic_rule_loop<program>{};
    static_assert(rl.simple("()()()"_cts) == "good"_cts);
    static_assert(rl.simple("()()))((()"_cts) == "bad"_cts);
}

TEST(monadic, augmented) {
    constexpr auto augmented = [](CtStr auto src) {
        return stateful{src, cumulative_effect{ [](int n, auto...) { return n+1; }, 0 }};
    };

    constexpr auto rl = monadic_rule_loop<program>{};

    constexpr auto res = rl(augmented("()()()"_cts));
    static_assert(res.value == "good"_cts);
    static_assert(res.aux.a == 4);
}

TEST(monadic, with_hidden) {
    constexpr auto rl_hidden = monadic_rule_loop< HIDDEN_RULE(FINAL_RULE("","")) >{};
    constexpr auto rl_public = monadic_rule_loop<             FINAL_RULE("","")  >{};
    static_assert(rl_hidden.simple(""_cts) == ""_cts);
    static_assert(rl_public.simple(""_cts) == ""_cts);

    constexpr auto augmented = [](CtStr auto src) {
        return stateful{src, cumulative_effect{ [](int n, auto...) { return n+1; }, 0 }};
    };

    constexpr auto res_hidden = rl_hidden(augmented(""_cts));
    static_assert(res_hidden.value == ""_cts);
    static_assert(res_hidden.aux.a == 0);

    constexpr auto res_public = rl_public(augmented(""_cts));
    static_assert(res_public.value == ""_cts);
    static_assert(res_public.aux.a == 1);
}

TEST(monadic, runtime) {
    constexpr auto rl = monadic_rule_loop<program>{};
    constexpr auto augmented = [](CtStr auto src) {
        return stateful{
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
