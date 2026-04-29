#pragma once

#include "concepts.h"
#include "ct.h"
#include "str.h"
#include "substitute.h"
#include "compose.h"
#include "augmented.h"

#include <iostream>
#include <iomanip>

// rule function types

// input: just CtStr
template<class T> concept RuleInput = CtStr<T> || Augmented<T>;

// output:
// - fail (not matched)
// - intermediate string (matched, stops processing rules, continues machine loop)
// - final string (matched, stops processing rules, stops machine loop)

struct fail {
    friend std::ostream& operator << (std::ostream& os, fail const& v) { os << "fail"; return os; }

    constexpr bool operator == (const fail&) const = default;
    constexpr bool operator == (const auto&) const { return false; }
};
constexpr bool failed(const auto& a) { return fail{} == a; }
template<class T> concept Fail = std::same_as<T, fail>;

// opposite to fail
template<class T> concept FailOrSubst = Fail<T> || CtStr<T>;

enum rule_state_t {
    regular_state,
    final_state,
};
template<class T> concept CtState = CtOf<T, rule_state_t>;

// successful result will be passed to the next loop.
// that's why type T is a kind of RuleInput
template<RuleInput T, CtState S> struct success {
    REPRESENTS(Success);
    T data;
    S state;  // TODO make it static!

    friend std::ostream& operator << (std::ostream& ost, const success& v) {
        return ost << "success{" << v.data1 << ", " << v.state1 << "}";
    }

    constexpr bool operator==(const success&) const = default;
    template<class T1, class S1>
    constexpr bool operator==(const success<T1, S1>&) const { return false; }
};
CONCEPT(Success);
constexpr bool finished(Success auto s) { return s.state.value == final_state; }

/// translate either ((either str str) str) to success (final / regular) / fail

#if 0
template<class T> concept NewRuleSuccess = Either<T> && RuleInput<typename T::type>;
template<class T> concept NewRuleSuccessOutput = Left<T> && NewRuleSuccess<typename T::type>;
template<class T> concept NewRuleFailOutput = Right<T> && RuleInput<typename T::type>;
template<class T> concept NewRuleOutput = NewRuleSuccessOutput<T> || NewRuleFailOutput<T>;

constexpr bool failed(NewRuleFailOutput auto const&) { return true; }
constexpr bool failed(NewRuleSuccessOutput auto const&) { return false; }

constexpr NewRuleSuccessOutput auto make_success(RuleInput auto r, CtState auto s) {
    if constexpr (s.value == regular_state)
        return left(right(r));
    else
        return left(left(r));
}
constexpr NewRuleFailOutput auto make_fail(RuleInput auto t) {
    return right(t);
}
constexpr RuleInput auto success_data(NewRuleSuccessOutput auto o) {
    return fromLeft(o).value;
}
constexpr CtState auto success_state(NewRuleSuccessOutput auto o) {
    return fromLeft(o).eitherConst(ct<final_state>{}, ct<regular_state>{});
}

template<class T> concept SuccessOutput = NewRuleSuccessOutput<T>;
template<class T> concept FailOutput = NewRuleFailOutput<T>;
template<class T> concept RuleOutput = NewRuleOutput<T>;
#else

template<class T> concept SuccessOutput = Success<T>;
template<class T> concept FailOutput = Fail<T>;
template<class T> concept RuleOutput = SuccessOutput<T> || FailOutput<T>;

constexpr auto make_success(RuleInput auto r, CtState auto s) { return success{r, s}; }
constexpr auto make_fail(RuleInput auto const& t) { return fail{}; }
constexpr auto success_data(Success auto const& s) { return s.data; }
constexpr auto success_state(Success auto const& s) { return s.state; }

#endif

// single search-and-replace function
CONCEPT(Rule)
CONCEPT(SingleRule)

template<Str auto s, Str auto r, rule_state_t state> struct rule {
    REPRESENTS(Rule)
    REPRESENTS(SingleRule)
    static_assert(
        !(state == regular_state && s == r),
        "same search and replace is meaningless: either inaccessible or results in an endless loop");

    static constexpr auto ct_search = ct<s>{};
    static constexpr auto ct_replace = ct<r>{};
    static constexpr auto ct_state = ct<state>{};

    friend std::ostream& operator << (std::ostream& os, rule const& v) {
        os
            << (state == regular_state ? "rule" : "final_rule")
            << "("
            << std::quoted(s.view())
            << " -> "
            << std::quoted(r.view())
            << ")";
        return os;
    }

    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        return try_substitute(ct_search, ct_replace, extract_text(t))
            .then_else(
                [&](CtStr auto res){
                    RuleInput auto tt = update_text(t, rule{}, res);
                    return make_success(tt, ct_state);
                },
                [&]{ return make_fail(t); }
            );
    }
};

// subroutine

namespace rules_helper {
    // implements disjunction on input >> rule1 >> rule2 >> ...

    constexpr auto make_arg(RuleInput auto input) { return right(input); }
    constexpr auto make_fun(Rule auto p) {
        // p   : input -> success | fail
        // fun : input -> right<input> | left<success>
        return [p](RuleInput auto t) -> Either auto {
            RuleOutput auto r = p(t);
            if constexpr (failed(r)) // not matched yet
                return right(t);     // continue...
            else
                return left(r);      // success
        };
    }
    constexpr RuleOutput auto take_res(Either auto a) {
        return a.either(
            [](auto r) { return r; }, // left the chain - success
            [](auto i) { return make_fail(i); } // went to the right - failed to match
        );
    }
} // namespace rules_helper

template<Rule auto... ps> struct rules {
    REPRESENTS(Rule)

    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        return rules_helper::take_res(
            (rules_helper::make_arg(t) >> ... >> rules_helper::make_fun(ps))
        );
    }
};

template<> struct rules<> {
    REPRESENTS(Rule)
    constexpr RuleOutput auto operator()(RuleInput auto t) const { return make_fail(t); }
};

// machine loop

namespace loop_helper {
    // implements endless loop on input >> rule >> rule >> ...

    constexpr auto make_arg(RuleInput auto input) { return right(input); }
    constexpr auto make_fun(Rule auto p) {
        // p   : input -> success{output,(regular|final)} | fail
        // fun : input -> left<output> | right<success>
        return [p](RuleInput auto t) {
            RuleOutput auto r = p(t);
            if constexpr (failed(r)) { // not matched anymore
                return left(make_success(t, ct<regular_state>{})); // stop with previous value
            } else {
                auto state = success_state(r); // r is not constexpr, so its state is not, too.
                if constexpr (state == ct<final_state>{}) // matched, final state
                    return left(r); // stop with result
                else
                    return right(success_data(r)); // matched, regular state - continue with new value
            }
        };
    }
    constexpr auto take_res(Either auto a) { return fromLeft(a); }

    template<class F> struct looped_fun {
        F f;
        constexpr auto operator()(RuleInput auto t) const {
            return right(t) >> f >> f >> f >> f >> f >> f >> f >> f >> *this;
        }
    };

} // namespace loop_helper

template<Rule auto p, size_t const factor = 10> struct rule_loop {
    REPRESENTS(Rule)

    // static constexpr auto the_loop = endless_loop{repeat<factor>(loop_helper::make_fun(p))};
    static constexpr auto the_loop = loop_helper::looped_fun{loop_helper::make_fun(p)};

    constexpr /* Success */ RuleOutput auto operator()(RuleInput auto t) const {
        return loop_helper::take_res(loop_helper::make_arg(t) >> the_loop);
    }
};

// machine is a function string -> string, without technical details
// about regular / final state.

template<Rule auto m> struct machine_fun {
    constexpr RuleInput auto operator()(RuleInput auto input) const {
        return success_data(m(input));
    }
};

// control augmentation

template<Rule auto p> struct hidden_rule {
    REPRESENTS(Rule)
    constexpr RuleOutput auto operator()(RuleInput auto t) const {
        RuleOutput auto out = p(extract_text(t)); // success(str, state) or fail
        if constexpr (failed(out)) {
            return make_fail(t);
        } else {
            return make_success(rebind_text(t, success_data(out)), success_state(out));
        }
    }
};

// useful macros

#define STR(s) (str{s}) // s##_ss
#define CTSTR(s) (ct<STR(s)>{}) // s##_cts
#define RULE(s, r) (rule<STR(s), STR(r), regular_state>{})
#define FINAL_RULE(s, r) (rule<STR(s), STR(r), final_state>{})
#define RULES(...) (rules<__VA_ARGS__>{})
#define RULE_LOOP(r) (rule_loop<(r)>{})
#define MACHINE_FROM_RULE(r) (machine_fun<(r)>{})
#define MACHINE(r) MACHINE_FROM_RULE(RULE_LOOP(r))

#define HIDDEN_RULE(p) (hidden_rule<(p)>{})

// To hide a program from compiler output
#define NAMED_RULE(name, p) \
(struct name { \
    REPRESENTS(Rule) \
    static constexpr auto impl = (p); \
    constexpr RuleOutput auto operator()(RuleInput auto t) const { return impl(t); } \
}){}
